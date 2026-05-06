// =====================================================================================
// M4D_AirPlaneCrash_Core.c - VERSÃO FINAL CONSOLIDADA (Suporte aos Novos Assets)
// Responsabilidade: Entidade âncora, Estado de Runtime, Snapshot Persistente e Ownership.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DO EVENTO (M4d_AirPlaneCrash)
	// ---------------------------------------------------------------------------------
	// Entidade âncora que representa a fuselagem do avião acidentado no mundo.
	// É responsável por manter o estado de Runtime, delegar o Snapshot Persistente
	// e gerenciar o Ownership (propriedade) de todos os objetos e ameaças ao seu redor.
	// ---------------------------------------------------------------------------------
class M4d_AirPlaneCrash extends CrashBase
{
	protected int  m_M4DEventID = 0;       
	protected int  m_CrashTier = 0;        
	protected bool m_M4DHasGas = false;    
	protected int  m_M4DElapsedMs = 0;     
	protected bool m_M4DKeyDropped = false; 
	
	protected bool m_HadWolves = false;
	protected bool m_HadBears = false;

	protected ref array<string> m_ZombieTypes;
	protected ref PlaneCrashAnimalSpawn m_WolfSpawn;
	protected ref PlaneCrashAnimalSpawn m_BearSpawn;

	protected ref array<EntityAI> m_DetailAssets;
	protected ref array<DayZInfected> m_Zombies;
	protected ref array<AnimalBase> m_Animals;
	protected ref array<EffectArea> m_GasAreas;
	protected EntityAI m_MainContainer;

	protected bool m_M4DTimerArmed = false;
	protected bool m_M4DIsTimerPaused = false;
	protected int  m_M4DPauseStartTime = 0;

	// Variáveis para a Narrativa Tática de Log
	protected int    m_M4DTotalPausedMs = 0;
	protected string m_M4DTriggerPlayerName = "";
	protected string m_M4DTriggerPlayerID = "";

	protected Particle m_PlaneCrash_FireEfx;

	static const int M4D_MAX_CONTAINER_LIFETIME = 1800000; 
	static const int M4D_CHECK_INTERVAL         = 100000;  

	// ---------------------------------------------------------------------------------
	// CONSTRUTOR DA ENTIDADE (Inicialização de Memória)
	// ---------------------------------------------------------------------------------
	// Chamado no momento em que o objeto é criado na engine. Prepara as listas (arrays)
	// para evitar Null Pointers e sorteia o ID único do evento no lado do servidor.
	// ---------------------------------------------------------------------------------
	void M4d_AirPlaneCrash()
	{
		m_DetailAssets = new array<EntityAI>();
		m_Zombies = new array<DayZInfected>();
		m_Animals = new array<AnimalBase>();
		m_GasAreas = new array<EffectArea>();
		
		if (GetGame() && GetGame().IsServer()) 
		{
			m_M4DEventID = Math.RandomIntInclusive(10000000, 99999999);
		}
	}

	// ---------------------------------------------------------------------------------
	// API DE ENCAPSULAMENTO (Getters e Setters)
	// ---------------------------------------------------------------------------------
	int GetEventID() { return m_M4DEventID; }
	int GetOwnerEventID() { return m_M4DEventID; }

	int GetDetailAssetsCount() { return m_DetailAssets.Count(); }
	int GetZombiesCount() { return m_Zombies.Count(); }
	int GetAnimalsCount() { return m_Animals.Count(); }
	int GetGasAreasCount() { return m_GasAreas.Count(); }

	bool HasKeyDropped() { return m_M4DKeyDropped; }
	void SetKeyDropped() { m_M4DKeyDropped = true; }

	// ---------------------------------------------------------------------------------
	// CONFIGURAÇÃO DE ESTADO (SetupEventState)
	// ---------------------------------------------------------------------------------
	void SetupEventState(int tier, bool hasGas) 
	{
		m_CrashTier = tier;
		m_M4DHasGas = hasGas;
		
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkWreckAlive(m_M4DEventID, GetPosition(), m_CrashTier, m_M4DHasGas);
		}
	}

	// ---------------------------------------------------------------------------------
	// CONFIGURAÇÃO DE AMEAÇAS (SetupThreatsData)
	// ---------------------------------------------------------------------------------
	void SetupThreatsData(array<string> zTypes, PlaneCrashAnimalSpawn wolf, PlaneCrashAnimalSpawn bear) 
	{
		if (zTypes) 
		{
			m_ZombieTypes = new array<string>();
			m_ZombieTypes.Copy(zTypes);
		}
		
		if (wolf && wolf.Enabled == 1) 
		{
			m_HadWolves = true;
		}
		
		if (bear && bear.Enabled == 1) 
		{
			m_HadBears = true;
		}
		
		m_WolfSpawn = wolf;
		m_BearSpawn = bear;
	}

	array<string> GetZombieTypes() { return m_ZombieTypes; }
	PlaneCrashAnimalSpawn GetWolfSpawn() { return m_WolfSpawn; }
	PlaneCrashAnimalSpawn GetBearSpawn() { return m_BearSpawn; }

	void SetMainContainer(EntityAI container) { m_MainContainer = container; }
	
	// ---------------------------------------------------------------------------------
	// GETTER DO CONTENTOR PRINCIPAL (GetMainContainer)
	// ---------------------------------------------------------------------------------
	// Permite que outras classes, como o gerenciador de ameaças, localizem a 
	// coordenada exata do baú para centralizar eventos (como o gás tóxico).
	// ---------------------------------------------------------------------------------
	EntityAI GetMainContainer() { return m_MainContainer; }
	
	void AddDetailAsset(EntityAI asset) 
	{ 
		if(asset) 
		{
			m_DetailAssets.Insert(asset); 
		}
	}
	
	void AddZombie(DayZInfected z) 
	{ 
		if(z) 
		{
			m_Zombies.Insert(z); 
		}
	}
	
	void AddAnimal(AnimalBase a) 
	{ 
		if(a) 
		{
			m_Animals.Insert(a); 
		}
	}
	
	void AddGasArea(EffectArea gas) 
	{ 
		if(gas) 
		{
			m_GasAreas.Insert(gas); 
		}
	}

	// ---------------------------------------------------------------------------------
	// INICIALIZAÇÃO DE ENGINE (EEInit)
	// ---------------------------------------------------------------------------------
	override void EEInit()
	{
		super.EEInit();
		
		if (GetGame() && GetGame().IsServer()) 
		{
			if (!m_M4DTimerArmed) 
			{ 
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_TimerTick, M4D_CHECK_INTERVAL, true); 
				m_M4DTimerArmed = true; 
				M4D_PlaneCrashLogger.TraceCallLater(m_M4DEventID, "Agendamento do M4D_TimerTick inicializado no boot da fuselagem.");
			}
			
			M4D_PlaneCrashLogger.EventStart(m_M4DEventID, "Fuselagem (Wreck) inicializada no mundo.");
		}
		
		if (GetGame() && !GetGame().IsDedicatedServer()) 
		{
			// As coordenadas relativas ao C130 permanecem blindadas e intocadas
			m_ParticleEfx = Particle.PlayOnObject(ParticleList.SMOKING_HELI_WRECK, this, Vector(4.7, -2.4, -2));
			m_PlaneCrash_FireEfx = Particle.PlayOnObject(ParticleList.BONFIRE_FIRE, this, Vector(4.7, -2.4, -2));
		}
	}

	// ---------------------------------------------------------------------------------
	// PERSISTÊNCIA E SNAPSHOT
	// ---------------------------------------------------------------------------------
	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_M4DEventID);
		ctx.Write(m_CrashTier); 
		ctx.Write(m_M4DHasGas);
		ctx.Write(m_M4DElapsedMs); 
		ctx.Write(m_M4DKeyDropped); 
		
		ctx.Write(m_HadWolves);
		ctx.Write(m_HadBears);
		
		if (m_ZombieTypes) 
		{
			ctx.Write(m_ZombieTypes.Count());
			for (int i = 0; i < m_ZombieTypes.Count(); i++) 
			{
				ctx.Write(m_ZombieTypes.Get(i));
			}
		} 
		else 
		{
			ctx.Write(0); 
		}

		ctx.Write(m_M4DTotalPausedMs);
		ctx.Write(m_M4DTriggerPlayerName);
		ctx.Write(m_M4DTriggerPlayerID);
		
		M4D_PlaneCrashLogger.TraceFunction(m_M4DEventID, "OnStoreSave executado. Estado do evento gravado no disco.");
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version)) 
		{
			return false;
		}
		
		if (!ctx.Read(m_M4DEventID) || m_M4DEventID == 0) 
		{
			m_M4DEventID = Math.RandomIntInclusive(10000000, 99999999);
		}
		
		if (!ctx.Read(m_CrashTier)) 
		{
			m_CrashTier = 0;
		}
		
		if (!ctx.Read(m_M4DHasGas)) 
		{
			m_M4DHasGas = false;
		}
		
		if (!ctx.Read(m_M4DElapsedMs)) 
		{
			m_M4DElapsedMs = 0;
		}
		
		if (!ctx.Read(m_M4DKeyDropped)) 
		{
			m_M4DKeyDropped = false; 
		}
		
		if (!ctx.Read(m_HadWolves)) 
		{
			m_HadWolves = false;
		}
		
		if (!ctx.Read(m_HadBears)) 
		{
			m_HadBears = false;
		}

		int zCount = 0;
		if (ctx.Read(zCount) && zCount > 0) 
		{
			m_ZombieTypes = new array<string>();
			for (int j = 0; j < zCount; j++) 
			{
				string zType;
				if (ctx.Read(zType)) 
				{
					m_ZombieTypes.Insert(zType);
				}
			}
		}

		if (!ctx.Read(m_M4DTotalPausedMs))
		{
			m_M4DTotalPausedMs = 0;
		}
		if (!ctx.Read(m_M4DTriggerPlayerName))
		{
			m_M4DTriggerPlayerName = "";
		}
		if (!ctx.Read(m_M4DTriggerPlayerID))
		{
			m_M4DTriggerPlayerID = "";
		}

		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkWreckAlive(m_M4DEventID, GetPosition(), m_CrashTier, m_M4DHasGas);

			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.RehydrateEvent, 1000, false);
			M4D_PlaneCrashLogger.TraceCallLater(m_M4DEventID, "Agendamento da reidratacao (RehydrateEvent) pos-load.");
			
			if (!m_M4DTimerArmed) 
			{ 
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_TimerTick, M4D_CHECK_INTERVAL, true); 
				m_M4DTimerArmed = true; 
			}
		}

		return true;
	}

	protected bool IsEventZombieType(string type)
	{
		if (type.Contains("ZmbM_NBC_")) return true;
		if (type.Contains("ZmbM_PatrolNormal_")) return true;
		if (type.Contains("ZmbM_SoldierNormal")) return true;
		if (type.Contains("ZmbM_usSoldier_")) return true;
		if (type.Contains("ZmbM_eastSoldier_")) return true;
		return false;
	}

	// ---------------------------------------------------------------------------------
	// RECUPERAÇÃO DE CENA (RehydrateEvent)
	// ---------------------------------------------------------------------------------
	protected void RehydrateEvent()
	{
		if (!GetGame() || !GetGame().IsServer()) 
		{
			return;
		}

		M4D_PlaneCrashLogger.TraceFunction(m_M4DEventID, "Iniciando RehydrateEvent. Recuperando posse das entidades...");

		m_DetailAssets.Clear();
		m_Zombies.Clear();
		m_Animals.Clear();
		m_GasAreas.Clear();
		m_MainContainer = null;

		array<Object> objs = new array<Object>();
		// CORREÇÃO DE VARREDURA: Raio otimizado para 50.0 metros para englobar perfeitamente a Wreck e o Contentor Principal.
		GetGame().GetObjectsAtPosition(GetPosition(), 120.0, objs, null);

		for (int i = 0; i < objs.Count(); i++) 
		{
			Object obj = objs.Get(i);
			if (!obj) 
			{
				continue;
			}
			
			string type = obj.GetType();

			if (type.Contains("M4D_WreckContainer") || type.Contains("M4DCrashStorage")) 
			{
				int objID = -1;
				GetGame().GameScript.CallFunction(obj, "GetOwnerEventID", objID, null);
				
				if (objID == m_M4DEventID) 
				{
					if (type.Contains("M4D_WreckContainer")) 
					{
						m_MainContainer = EntityAI.Cast(obj);
					} 
					else 
					{
						m_DetailAssets.Insert(EntityAI.Cast(obj));
					}
				}
				continue;
			}

			// EXPANSÃO TÁTICA: Reconhecimento dos novos assets do JSON no restart.
			// Evita que os novos destroços (veículos, guindastes e crateras) fiquem órfãos e causem fuga de memória.
			if (type == "CraterLong" || type == "StaticObj_ammoboxes_big" || type.Contains("Land_Wreck_") || type.Contains("Land_wreck_") || type.Contains("StaticObj_ShellCrater") || type.Contains("enginecrane") || type.Contains("pallettrolley")) 
			{
				if (vector.Distance(GetPosition(), obj.GetPosition()) <= 35.0) 
				{
					m_DetailAssets.Insert(EntityAI.Cast(obj));
				}
				continue;
			}

			if (DayZInfected.Cast(obj)) 
			{
				if (vector.Distance(GetPosition(), obj.GetPosition()) <= 45.0 && IsEventZombieType(type)) 
				{
					m_Zombies.Insert(DayZInfected.Cast(obj));
				}
				continue;
			}

			if (AnimalBase.Cast(obj)) 
			{
				string aType = type; 
				aType.ToLower();
				if (m_HadWolves && aType.Contains("wolf")) 
				{
					m_Animals.Insert(AnimalBase.Cast(obj));
				} 
				else if (m_HadBears && aType.Contains("bear")) 
				{
					m_Animals.Insert(AnimalBase.Cast(obj));
				}
				continue;
			}

			if (type == "ContaminatedArea_Static" || type == "ContaminatedArea_Dynamic") 
			{
				if (vector.Distance(GetPosition(), obj.GetPosition()) <= 50.0) 
				{
					m_GasAreas.Insert(EffectArea.Cast(obj));
				}
				continue;
			}
		}

		// ---------------------------------------------------------------------------------
		// AUTO-HEAL DE GÁS TÓXICO
		// ---------------------------------------------------------------------------------
		// Zonas de gás criadas dinamicamente via script não persistem nativamente no 
		// banco de dados (DB) do DayZ após o reinício do servidor. Se o log do snapshot
		// diz que havia gás (m_M4DHasGas), e ele sumiu, nós o recriamos aqui instantaneamente
		// na posição do contêiner, blindando o ecossistema.
		// ---------------------------------------------------------------------------------
		if (m_M4DHasGas == true)
		{
			if (m_GasAreas.Count() == 0)
			{
				vector gasPos = GetPosition();
				if (m_MainContainer)
				{
					gasPos = m_MainContainer.GetPosition();
				}

				EffectArea gas = EffectArea.Cast(GetGame().CreateObject("ContaminatedArea_Static", gasPos));
				if (gas)
				{
					gas.PlaceOnSurface();
					m_GasAreas.Insert(gas);
					M4D_PlaneCrashLogger.TraceSpawn(m_M4DEventID, "AUTO-HEAL: Zona de Gas recriada com sucesso pos-restart.");
				}
			}
		}

		if (m_MainContainer) 
		{
			M4D_PlaneCrashWorldState.MarkContainerAlive(m_M4DEventID, m_MainContainer.GetType());

			bool isUnlocked = false;
			GetGame().GameScript.CallFunction(m_MainContainer, "GetUnlockedState", isUnlocked, null);
			
			if (isUnlocked) 
			{
				M4D_PlaneCrashWorldState.MarkContainerUnlocked(m_M4DEventID);
			} 
			else 
			{
				string key = "ShippingContainerKeys_Blue";
				string cType = m_MainContainer.GetType();
				if (cType.Contains("Red")) key = "ShippingContainerKeys_Red";
				else if (cType.Contains("Yellow")) key = "ShippingContainerKeys_Yellow";
				else if (cType.Contains("Orange")) key = "ShippingContainerKeys_Orange";

				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(M4D_PlaneCrashThreats.M4D_SpawnZombiesWithKey, 2000, false, this, m_MainContainer.GetPosition(), key);
			}
		}

		vector wMat[3];
		M4D_PlaneCrashSpawner.BuildWreckMatrix(GetPosition(), GetDirection(), wMat);
		M4D_PlaneCrashSpawner.SpawnDetailAssets(this, wMat, m_CrashTier, true); 

		M4D_PlaneCrashWorldState.UpdateCounts(m_M4DEventID, m_DetailAssets.Count(), m_Zombies.Count(), m_Animals.Count(), m_GasAreas.Count());
		M4D_PlaneCrashWorldState.MarkRestored(m_M4DEventID);
		
		M4D_PlaneCrashLogger.TraceFunction(m_M4DEventID, "RehydrateEvent concluido. Posse restabelecida.");
	}

	// ---------------------------------------------------------------------------------
	// ROTINA DE DELEÇÃO E LIMPEZA (EEDelete)
	// ---------------------------------------------------------------------------------
	// Engatilha o expurgo da RAM. Aqui injetamos o Bloco Tático [EVENT CLOSED],
	// calculando matematicamente a duração consolidada (total, ativo e pausado)
	// e informando os status finais sem usar nenhum operador ternário.
	// ---------------------------------------------------------------------------------
	override void EEDelete(EntityAI parent)
	{
		if (GetGame() && GetGame().IsServer()) 
		{
			// ====== BLOCO TÁTICO: EVENT CLOSED & DURAÇÃO CONSOLIDADA ======
			// Matematica precisa do tempo estrito
			int activeMs = m_M4DElapsedMs;
			int pausedMs = m_M4DTotalPausedMs;
			int totalMs = activeMs + pausedMs;

			int totalMins = (totalMs / 1000) / 60;
			int totalSecs = (totalMs / 1000) % 60;

			int activeMins = (activeMs / 1000) / 60;
			int activeSecs = (activeMs / 1000) % 60;

			int pausedMins = (pausedMs / 1000) / 60;
			int pausedSecs = (pausedMs / 1000) % 60;

			bool isUnlocked = false;
			if (m_MainContainer) 
			{
				GetGame().GameScript.CallFunction(m_MainContainer, "GetUnlockedState", isUnlocked, null);
			}
			
			string unlockedStr = "false";
			if (isUnlocked == true) 
			{
				unlockedStr = "true";
			}

			// Correção da limitação do string.Format (Máximo 9 parâmetros no Enforce Script).
			// Fatiamento da string principal para garantir a compatibilidade e a estabilidade.
			string closeLogPart1 = string.Format("\n[EVENT CLOSED]\nEventID: %1\nTotalDuration: %2m %3s\n", m_M4DEventID, totalMins, totalSecs);
			string closeLogPart2 = string.Format("ActiveTime: %1m %2s\nPausedTime: %3m %4s\nReason: lifetime expired / cleanup\n", activeMins, activeSecs, pausedMins, pausedSecs);
			string closeLogPart3 = string.Format("FinalCounts:\nAssets=%1\nZombies=%2\nAnimals=%3\n", m_DetailAssets.Count(), m_Zombies.Count(), m_Animals.Count());
			string closeLogPart4 = string.Format("Gas=%1\nContainerUnlocked=%2\nChestSpawned=true", m_GasAreas.Count(), unlockedStr);
			
			string closeLog = closeLogPart1 + closeLogPart2 + closeLogPart3 + closeLogPart4;
			
			M4D_PlaneCrashLogger.EventClose(m_M4DEventID, closeLog);
			// ==============================================================
			
			M4D_PlaneCrashLogger.TraceOwnership(m_M4DEventID, "Iniciando exclusao das entidades atreladas por Ownership...");
			
			M4D_PlaneCrashWorldState.CloseEvent(m_M4DEventID);
			
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.M4D_TimerTick);
			
			if (m_MainContainer) 
			{ 
				int cID = -1; 
				GetGame().GameScript.CallFunction(m_MainContainer, "GetOwnerEventID", cID, null);
				if (cID == m_M4DEventID || cID == -1) 
				{
					GetGame().ObjectDelete(m_MainContainer);
				}
			}
			
			foreach (EntityAI asset : m_DetailAssets) 
			{
				if (asset) 
				{ 
					int aID = -1; 
					GetGame().GameScript.CallFunction(asset, "GetOwnerEventID", aID, null);
					if (aID == m_M4DEventID || aID == -1) 
					{
						GetGame().ObjectDelete(asset);
					}
				}
			}
			
			foreach (DayZInfected z : m_Zombies) 
			{ 
				if (z) 
				{
					GetGame().ObjectDelete(z); 
				}
			}
			
			foreach (AnimalBase a : m_Animals) 
			{ 
				if (a) 
				{
					GetGame().ObjectDelete(a); 
				}
			}
			
			foreach (EffectArea gas : m_GasAreas) 
			{ 
				if (gas) 
				{
					GetGame().ObjectDelete(gas); 
				}
			}

			m_DetailAssets.Clear(); m_Zombies.Clear(); m_Animals.Clear(); m_GasAreas.Clear(); m_MainContainer = null;
		}
		
		if (GetGame() && !GetGame().IsDedicatedServer()) 
		{
			if (m_ParticleEfx) m_ParticleEfx.Stop();
			if (m_PlaneCrash_FireEfx) m_PlaneCrash_FireEfx.Stop();
		}
		
		super.EEDelete(parent);
	}

	// ---------------------------------------------------------------------------------
	// RELÓGIO DE CICLO DE VIDA E RADAR DE PLAYERS (M4D_TimerTick)
	// ---------------------------------------------------------------------------------
	// Controlador do batimento cardíaco da fuselagem e guardião do tempo. Implementa a
	// lógica de pausa quando jogadores se aproximam e gera os blocos verticais de tracking
	// (Entrada, Permanência e Saída do Player) como poderoso sistema anti-exploit.
	// ---------------------------------------------------------------------------------
	protected void M4D_TimerTick() 
	{
		if (!GetGame() || !GetGame().IsServer()) 
		{
			return;
		}

		M4D_PlaneCrashWorldState.Heartbeat(m_M4DEventID);

		vector pos = GetPosition();
		array<Man> players = new array<Man>(); 
		GetGame().GetPlayers(players);
		
		bool anyPlayerNearby = false;
		PlayerBase triggeringPlayer = null;

		// Radar passa a rastrear instâncias de jogadores vivos dentro do raio
		for (int i = 0; i < players.Count(); i++) 
		{
			Man p = players.Get(i);
			if (p) 
			{
				if (vector.Distance(pos, p.GetPosition()) <= 1000.0) 
				{ 
					anyPlayerNearby = true; 
					triggeringPlayer = PlayerBase.Cast(p);
					break; 
				}
			}
		}

		if (anyPlayerNearby == true) 
		{
			if (m_M4DIsTimerPaused == false) 
			{
				m_M4DIsTimerPaused = true; 
				m_M4DPauseStartTime = GetGame().GetTime() / 1000;

				if (triggeringPlayer)
				{
					if (triggeringPlayer.GetIdentity())
					{
						m_M4DTriggerPlayerName = triggeringPlayer.GetIdentity().GetName();
						m_M4DTriggerPlayerID = triggeringPlayer.GetIdentity().GetId();
					}
					else
					{
						m_M4DTriggerPlayerName = "Desconhecido";
						m_M4DTriggerPlayerID = "N/A";
					}
				}
				else
				{
					m_M4DTriggerPlayerName = "Desconhecido";
					m_M4DTriggerPlayerID = "N/A";
				}

				// ====== BLOCO TÁTICO: PLAYER ENTERED ======
				string enterLog = string.Format("\n[PLAYER ENTERED]\nEventID: %1\nPlayer: %2\nSteamID: %3\nAction: Timer Paused", m_M4DEventID, m_M4DTriggerPlayerName, m_M4DTriggerPlayerID);
				M4D_PlaneCrashLogger.EventPlayer(m_M4DEventID, enterLog);
				// ==========================================
			}
		} 
		else 
		{
			if (m_M4DIsTimerPaused == true) 
			{
				m_M4DIsTimerPaused = false;

				int pauseDurationSeconds = (GetGame().GetTime() / 1000) - m_M4DPauseStartTime;
				m_M4DTotalPausedMs = m_M4DTotalPausedMs + (pauseDurationSeconds * 1000);

				// ====== BLOCO TÁTICO: PLAYER LEFT ======
				string leaveLog = string.Format("\n[PLAYER LEFT]\nEventID: %1\nPlayer: %2\nSteamID: %3\nAction: Timer Resumed\nPausedDuration: %4s", m_M4DEventID, m_M4DTriggerPlayerName, m_M4DTriggerPlayerID, pauseDurationSeconds);
				M4D_PlaneCrashLogger.EventPlayer(m_M4DEventID, leaveLog);
				// =======================================

				m_M4DTriggerPlayerName = "";
				m_M4DTriggerPlayerID = "";
			}
			
			m_M4DElapsedMs += M4D_CHECK_INTERVAL;
			
			if (m_M4DElapsedMs >= M4D_MAX_CONTAINER_LIFETIME) 
			{
				GetGame().ObjectDelete(this);
			}
		}
	}

	// ---------------------------------------------------------------------------------
	// ATUALIZADOR FÍSICO DE TERRENO (M4D_UpdateNavmesh)
	// ---------------------------------------------------------------------------------
	static void M4D_UpdateNavmesh(Object obj) 
	{ 
		if (obj && GetGame() && GetGame().IsServer()) 
		{
			GetGame().UpdatePathgraphRegionByObject(obj); 
		}
	}
}