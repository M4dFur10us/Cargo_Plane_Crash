// =====================================================================================
// M4D_MissionServer.c - VERSÃO FINAL CONSOLIDADA (Garbage Collector e Lista Branca)
// Responsabilidade: Inicialização do Mod, Gestão de Timers e Varredura Centralizada.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DO SERVIDOR (MissionServer)
	// ---------------------------------------------------------------------------------
	// Modificação da classe núcleo do DayZ. Responsável por inicializar os módulos 
	// do mod, gerenciar o timer assíncrono para novos eventos e comandar limpezas 
	// globais profundas baseadas em Ownership caso o mod seja desativado.
	// ---------------------------------------------------------------------------------
modded class MissionServer
{
	// Variáveis de controle de tempo (em Segundos)
	protected int m_M4D_NextSpawnTime = 0;
	protected bool m_M4D_IsSystemInitialized = false;

	// ---------------------------------------------------------------------------------
	// INICIALIZAÇÃO DO SISTEMA (Ponto de Entrada do Servidor)
	// ---------------------------------------------------------------------------------
	// Este método é executado uma única vez quando o servidor inicia. Ele carrega 
	// as configurações JSON e decide se o sistema de agendamento deve começar ou 
	// se deve executar o protocolo de limpeza caso o mod tenha sido desativado.
	// ---------------------------------------------------------------------------------
	override void OnInit()
	{
		super.OnInit();

		if (!m_M4D_IsSystemInitialized) 
		{
			M4D_PlaneCrashLogger.Info("========== INICIANDO M4D AIRPLANE CRASH SYSTEM ==========");
			
			ref M4D_PlaneCrashSettings settings = M4D_PlaneCrashSettings.Get();
			M4D_PlaneCrashSites.Get();
			M4D_PlaneCrashLoot.Get();
			
			// Aciona o WorldState para garantir que a RAM seja hidratada (via JSON ou BKP)
			M4D_PlaneCrashWorldState.EnsureInit();

			if (settings) 
			{
				// MASTER SWITCH: Se EnableMod for 0, cancela o agendamento e limpa o mapa
				if (settings.EnableMod == 0) 
				{
					M4D_PlaneCrashLogger.Warn("[MissionServer] Mod M4D Airplane Crash esta DESLIGADO. Iniciando protocolo de limpeza...");
					
					// Agenda a limpeza centralizada para 15 segundos após o boot
					GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_ExecuteMasterCleanup, 15000, false);
					
					m_M4D_IsSystemInitialized = true;
					return; 
				}

				// AÇÃO 4: GARBAGE COLLECTOR SUPREMO (Protocolo de Lista Branca)
				M4D_ExecuteOrphanCleanup();

				// Define o tempo para o primeiro evento (60 segundos após o boot)
				m_M4D_NextSpawnTime = (GetGame().GetTime() / 1000) + 60;
				M4D_PlaneCrashLogger.Info("[MissionServer] Sistema ativo. Primeiro evento em 60 segundos.");
			}

			// Inicia o Tick de verificação a cada 5 segundos
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_EventTick, 5000, true);
			
			m_M4D_IsSystemInitialized = true;
		}
	}

	// ---------------------------------------------------------------------------------
	// PROTOCOLO DE LIMPEZA CENTRALIZADA (Varredura por Ownership)
	// ---------------------------------------------------------------------------------
	// Se o mod for desligado, esta função usa o último Snapshot salvo para localizar 
	// as coordenadas de todos os eventos ativos e deletar cirurgicamente aviões, 
	// containers e os novos baús físicos, comparando o EventID de cada objeto.
	// ---------------------------------------------------------------------------------
	void M4D_ExecuteMasterCleanup()
	{
		if (!GetGame() || !GetGame().IsServer()) 
		{
			return;
		}

		M4D_PlaneCrashLogger.Warn("[MissionServer] [Master Switch] Executando varredura centralizada por OWNERSHIP...");

		string snapPath = "$profile:M4D_AirPlaneCrash/WorldStateSnapshot.json";
		
		if (FileExist(snapPath)) 
		{
			ref M4D_WorldStateSnapshot snap = new M4D_WorldStateSnapshot();
			string errorMessage;
			
			if (JsonFileLoader<M4D_WorldStateSnapshot>.LoadFile(snapPath, snap, errorMessage)) 
			{
				if (snap.TrackedEvents) 
				{
					for (int i = 0; i < snap.TrackedEvents.Count(); i++) 
					{
						M4D_EventState state = snap.TrackedEvents.Get(i);
						if (state) 
						{
							int targetEventID = state.EventID;
							vector pos = state.WreckPosition;
							array<Object> objs = new array<Object>();
							
							GetGame().GetObjectsAtPosition(pos, 200.0, objs, null);
							
							int deletedObjs = 0;
							for (int j = 0; j < objs.Count(); j++) 
							{
								Object obj = objs.Get(j);
								if (!obj) 
								{
									continue;
								}
								
								string type = obj.GetType();
								int foundID = -1;

								// Identifica e captura o ID de propriedade de acordo com a classe
								if (type == "M4d_AirPlaneCrash") 
								{
									GetGame().GameScript.CallFunction(obj, "GetEventID", foundID, null);
								} 
								else if (type.Contains("M4D_WreckContainer") || type == "M4D_CrashRewardChest" || type.Contains("M4DCrashStorage")) 
								{
									GetGame().GameScript.CallFunction(obj, "GetOwnerEventID", foundID, null);
								}
								
								// Só deleta se o ID bater com o evento rastreado no Snapshot
								if (foundID == targetEventID) 
								{
									GetGame().ObjectDelete(obj);
									deletedObjs++;
								}
								// Limpeza de crateras e lixo visual estático associado
								else if (foundID == -1 && (type == "CraterLong" || type == "StaticObj_ammoboxes_big"))
								{
									if (vector.Distance(pos, obj.GetPosition()) <= 40.0) 
									{
										GetGame().ObjectDelete(obj);
										deletedObjs++;
									}
								}
							}
							M4D_PlaneCrashLogger.Info(string.Format("[MissionServer] EventID %1: %2 entidades deletadas.", targetEventID, deletedObjs));
						}
					}
				}
			}
			
			DeleteFile(snapPath);
			M4D_PlaneCrashLogger.Info("[MissionServer] Protocolo Master Switch concluido. Persistencia apagada.");
		} 
	}

	// ---------------------------------------------------------------------------------
	// GARBAGE COLLECTOR DE BOOT (M4D_ExecuteOrphanCleanup)
	// ---------------------------------------------------------------------------------
	// Varre o mapa Chernarus/Livonia em busca de fuselagens órfãs (Ação 4). 
	// Utiliza o protocolo de Lista Branca: se o ID não estiver na RAM (vinda do 
	// Snapshot principal ou BKP), o objeto é considerado lixo e removido do mundo.
	// ---------------------------------------------------------------------------------
	void M4D_ExecuteOrphanCleanup()
	{
		if (!GetGame() || !GetGame().IsServer()) 
		{
			return;
		}

		M4D_PlaneCrashLogger.Info("[MissionServer] Iniciando Varredura Global de seguranca (Acao 4)...");

		// Centro aproximado do mapa com raio massivo para cobrir Chernarus/Livonia (15km)
		vector mapCenter = Vector(7500, 0, 7500); 
		array<Object> worldObjects = new array<Object>();
		GetGame().GetObjectsAtPosition(mapCenter, 15000.0, worldObjects, null);

		int cleanedCount = 0;
		int preservedCount = 0;

		for (int i = 0; i < worldObjects.Count(); i++)
		{
			Object obj = worldObjects.Get(i);
			if (!obj) 
			{
				continue;
			}

			if (obj.GetType() == "M4d_AirPlaneCrash")
			{
				int wreckID = -1;
				GetGame().GameScript.CallFunction(obj, "GetEventID", wreckID, null);

				// PROTOCOLO DE LISTA BRANCA:
				if (wreckID != -1)
				{
					if (M4D_PlaneCrashWorldState.HasEvent(wreckID))
					{
						preservedCount++;
						M4D_PlaneCrashLogger.Info(string.Format("[MissionServer] Evento VALIDO detectado: ID %1 preservado.", wreckID));
					}
					else
					{
						cleanedCount++;
						M4D_PlaneCrashLogger.Warn(string.Format("[MissionServer] ORFAO DETECTADO: ID %1 nao consta no Snapshot. Pulverizando...", wreckID));
						GetGame().ObjectDelete(obj);
					}
				}
			}
		}

		M4D_PlaneCrashLogger.Info(string.Format("[MissionServer] Varredura concluida. Orfaos limpos: %1 | Eventos preservados: %2", cleanedCount, preservedCount));
	}

	// ---------------------------------------------------------------------------------
	// RELÓGIO DO AGENDADOR (O Coração do Tick)
	// ---------------------------------------------------------------------------------
	// Executado a cada 5 segundos, este método monitora a integridade do WorldState 
	// e decide se é o momento de gerar um novo avião no mapa, respeitando o limite 
	// máximo de eventos ativos e a cadência fixa de agendamento.
	// ---------------------------------------------------------------------------------
	void M4D_EventTick()
	{
		if (!GetGame() || !GetGame().IsServer()) 
		{
			return;
		}

		// Garante a auditoria e limpeza de lixo na RAM a cada ciclo
		M4D_PlaneCrashWorldState.AuditAndClean();

		int currentUnixTime = GetGame().GetTime() / 1000; 
		
		if (currentUnixTime >= m_M4D_NextSpawnTime) 
		{
			ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
			if (!s) 
			{
				return;
			}

			int activeValidEvents = M4D_PlaneCrashWorldState.GetValidActiveCrashCount();
			
			if (activeValidEvents < s.MaxActivePlaneEvents) 
			{
				// ====== BLOCO TÁTICO: EVENT START REQUEST ======
				string requestLog = "\n[EVENT START REQUEST]\nEventID: pendente\nTrigger: MissionServer / AutoCycle\nReason: Slot disponivel para novo evento";
				M4D_PlaneCrashLogger.EventStart(0, requestLog);
				// ===============================================

				M4D_PlaneCrashLogger.Info(string.Format("[MissionServer] Autorizando Spawn. Ativos: %1/%2", activeValidEvents, s.MaxActivePlaneEvents));
				
				M4D_PlaneCrashSpawner.SpawnSite();
				
				// CADÊNCIA FIXA: Próximo agendamento em 30 segundos
				m_M4D_NextSpawnTime = currentUnixTime + 30;
				
				M4D_PlaneCrashLogger.Info("[MissionServer] Proximo evento em 30 segundos.");
			} 
		}
	}
}