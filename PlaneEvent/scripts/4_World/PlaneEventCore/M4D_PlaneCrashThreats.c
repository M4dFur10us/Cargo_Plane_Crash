// =====================================================================================
// M4D_PlaneCrashThreats.c - VERSÃO DEFINITIVA (Controle Estrito de Cota e Validação JSON)
// Responsabilidade: Gestão e Spawn de Zumbis, Lobos, Ursos e Gás baseado no Snapshot.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE GERENCIADORA DE AMEAÇAS (M4D_PlaneCrashThreats)
	// ---------------------------------------------------------------------------------
	// Motor secundário acionado pelo Spawner. É responsável por povoar o perímetro do
	// acidente com NPCs e Gás. Além disso, executa o algoritmo "Anti-Farming", que 
	// garante que apenas uma chave do contentor seja injetada no inventário de um zumbi.
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashThreats
{
	// ---------------------------------------------------------------------------------
	// ZUMBIS E INJEÇÃO DA CHAVE DO CONTENTOR
	// ---------------------------------------------------------------------------------
	static void M4D_SpawnZombiesWithKey(M4d_AirPlaneCrash wreck, vector center, string keyName)
	{
		if (!wreck) 
		{
			return;
		}

		ref M4D_PlaneCrashSettings settings = M4D_PlaneCrashSettings.Get();
		if (!settings) 
		{
			return;
		}

		// 1. Radar passivo de Gás para decidir a máscara/roupa de proteção dos Zumbis
		bool isGasActive = false;
		array<Object> envObjs = new array<Object>();
		GetGame().GetObjectsAtPosition(center, 60.0, envObjs, null);
		
		for (int e = 0; e < envObjs.Count(); e++) 
		{
			if (envObjs.Get(e))
			{
				if (envObjs.Get(e).GetType() == "ContaminatedArea_Static") 
				{
					isGasActive = true;
					break;
				}
			}
		}

		// 2. Conta os zumbis já existentes na área para não sobrecarregar a performance do servidor
		int currentZombies = 0;
		array<Object> objs = new array<Object>();
		GetGame().GetObjectsAtPosition(center, 45.0, objs, null);
		
		for (int i = 0; i < objs.Count(); i++) 
		{
			if (DayZInfected.Cast(objs.Get(i))) 
			{
				currentZombies++;
			}
		}

		// Validação rigorosa da cota pelo administrador via JSON com teto arquitetônico
		int targetZombies = settings.ZombieCount;
		if (targetZombies > 20) 
		{
			targetZombies = 20;
		}
		if (targetZombies < 0) 
		{
			targetZombies = 0;
		}

		// Resolução do Paradoxo Lógico: Zumbi Zero sobrepõe a configuração da chave
		bool allowKeyDrop = false;
		if (settings.EnableZombieKeyDrop == 1) 
		{
			if (targetZombies > 0) 
			{
				allowKeyDrop = true;
			}
			else 
			{
				M4D_PlaneCrashLogger.Warn(string.Format("[Threats] ZombieCount eh 0 mas EnableZombieKeyDrop eh 1. KeyDrop anulado por seguranca (EventID: %1).", wreck.GetEventID()));
			}
		}

		int toSpawn = targetZombies - currentZombies;
		
		if (toSpawn < 0) 
		{
			toSpawn = 0;
		}
		
		// Garante a existência do portador da chave caso a área atinja o teto pelas sobras do mapa
		if (toSpawn == 0 && allowKeyDrop == true && wreck.HasKeyDropped() == false) 
		{
			toSpawn = 1;
		}

		if (toSpawn == 0) 
		{
			return; 
		}

		array<string> zTypes = wreck.GetZombieTypes();
		if (!zTypes || zTypes.Count() == 0) 
		{
			zTypes = new array<string>();
			zTypes.Insert("ZmbM_PatrolNormal_PautRev");
			zTypes.Insert("ZmbM_SoldierNormal");
		}

		array<DayZInfected> spawnedZeds = new array<DayZInfected>();

		// 3. Spawna os Zumbis
		for (int s = 0; s < toSpawn; s++) 
		{
			string zType = zTypes.GetRandomElement();
			
			// Se o avião caiu numa zona de gás, força o spawn de zumbis com traje NBC
			if (isGasActive == true)
			{
				if (!zType.Contains("NBC")) 
				{
					zType = "ZmbM_NBC_Yellow"; 
				}
			}
			
			vector zPos = center + Vector(Math.RandomFloat(-15.0, 15.0), 0, Math.RandomFloat(-15.0, 15.0));
			zPos[1] = GetGame().SurfaceY(zPos[0], zPos[2]);
			
			DayZInfected zed = DayZInfected.Cast(GetGame().CreateObject(zType, zPos, false, true));
			if (zed) 
			{
				wreck.AddZombie(zed);
				spawnedZeds.Insert(zed);
			}
		}

		M4D_PlaneCrashLogger.TraceSpawn(wreck.GetEventID(), string.Format("Zumbis -> Cota JSON: %1 | Spawns gerados com sucesso: %2", targetZombies, spawnedZeds.Count()));

		// 4. A Roleta da Chave (Proteção Anti-Farming)
		// Verifica as regras estritas da chave antes da inserção
		if (allowKeyDrop == true && spawnedZeds.Count() > 0 && wreck.HasKeyDropped() == false) 
		{
			DayZInfected luckyZed = spawnedZeds.GetRandomElement();
			if (luckyZed)
			{
				if (luckyZed.GetInventory()) 
				{
					EntityAI keyItem = luckyZed.GetInventory().CreateInInventory(keyName);
					if (keyItem) 
					{
						// Blinda o evento para não dropar mais chaves no futuro
						wreck.SetKeyDropped();
						
						// INTEGRAÇÃO WORLDSTATE: Comunica ao Painel Vivo que o evento forneceu a chave
						M4D_PlaneCrashWorldState.MarkKeyDropped(wreck.GetEventID());
						
						M4D_PlaneCrashLogger.TraceObject(wreck.GetEventID(), string.Format("Protecao Anti-Farming: Chave '%1' gerada e bloqueada para futuros spawns.", keyName));
					}
				}
			}
		}

		// ====== BLOCO TÁTICO: THREATS SPAWNED ======
		// Neste momento (3000ms após o Spawner), os Animais e o Gás (1000ms) já foram criados.
		// Coletamos a contagem final exata do wreck para montar a auditoria do administrador.
		string keyDroppedStr = "no";
		if (wreck.HasKeyDropped() == true)
		{
			keyDroppedStr = "yes";
		}

		string threatsLog = string.Format("\n[THREATS SPAWNED]\nEventID: %1\nZombies: %2\nAnimals: %3\nGasAreas: %4\nKeyDropped: %5", wreck.GetEventID(), wreck.GetZombiesCount(), wreck.GetAnimalsCount(), wreck.GetGasAreasCount(), keyDroppedStr);
		M4D_PlaneCrashLogger.EventThreats(wreck.GetEventID(), threatsLog);
		// ===========================================
	}

	// ---------------------------------------------------------------------------------
	// LOBOS, URSOS E ZONAS DE GÁS
	// ---------------------------------------------------------------------------------
	static void SpawnThreatsInstance(M4d_AirPlaneCrash wreck, bool isGasSpawning)
	{
		if (!wreck) 
		{
			return;
		}
		
		vector center = wreck.GetPosition();
		
		// 1. Instanciação do Gás (Centralização no Contêiner)
		if (isGasSpawning) 
		{
			// Iniciamos com a posição do avião como segurança
			vector gasPos = center;
			
			// Tentamos localizar o contêiner principal vinculado ao evento
			EntityAI mainContainer = wreck.GetMainContainer();
			if (mainContainer)
			{
				// O epicentro do gás passa a ser a coordenada exata do contêiner
				gasPos = mainContainer.GetPosition();
			}

			EffectArea gas = EffectArea.Cast(GetGame().CreateObject("ContaminatedArea_Static", gasPos));
			if (gas) 
			{
				gas.PlaceOnSurface();
				wreck.AddGasArea(gas);
				M4D_PlaneCrashLogger.TraceSpawn(wreck.GetEventID(), "Zona de Gas centralizada com sucesso sobre o contentor principal.");
			}
		}

		int wolfCount = 0;
		int bearCount = 0;

		// Funil Matemático de Cota de Animais (Teto Máximo Combinado = 20)
		int maxAnimalsAllowed = 20;
		
		int desiredWolves = 0;
		PlaneCrashAnimalSpawn wolf = wreck.GetWolfSpawn();
		if (wolf)
		{
			if (wolf.Enabled == 1)
			{
				desiredWolves = wolf.Count;
			}
		}
		
		int desiredBears = 0;
		PlaneCrashAnimalSpawn bear = wreck.GetBearSpawn();
		if (bear)
		{
			if (bear.Enabled == 1)
			{
				desiredBears = bear.Count;
			}
		}
		
		if (desiredWolves > maxAnimalsAllowed) 
		{
			desiredWolves = maxAnimalsAllowed;
		}
		
		int remainingForBears = maxAnimalsAllowed - desiredWolves;
		if (desiredBears > remainingForBears) 
		{
			desiredBears = remainingForBears;
		}

		// 2. Spawn de Lobos (Carga baseada no Teto e nas definições do JSON)
		if (desiredWolves > 0)
		{
			if (wolf.Types && wolf.Types.Count() > 0) 
			{
				for (int i = 0; i < desiredWolves; i++) 
				{
					string wolfType = wolf.Types.GetRandomElement();
					vector wPos = Vector(center[0] + Math.RandomFloat(-wolf.Radius, wolf.Radius), 0, center[2] + Math.RandomFloat(-wolf.Radius, wolf.Radius));
					wPos[1] = GetGame().SurfaceY(wPos[0], wPos[2]);
					
					AnimalBase wAnimal = AnimalBase.Cast(GetGame().CreateObject(wolfType, wPos, false, true));
					if (wAnimal) 
					{
						// ENTREGA O RECIBO AO WRECK
						wreck.AddAnimal(wAnimal);
						wolfCount++;
					}
				}
			}
		}
		
		// 3. Spawn de Ursos (Carga baseada nas sobras do Teto e no JSON)
		if (desiredBears > 0)
		{
			if (bear.Types && bear.Types.Count() > 0) 
			{
				for (int j = 0; j < desiredBears; j++) 
				{
					string bearType = bear.Types.GetRandomElement();
					vector bPos = Vector(center[0] + Math.RandomFloat(-bear.Radius, bear.Radius), 0, center[2] + Math.RandomFloat(-bear.Radius, bear.Radius));
					bPos[1] = GetGame().SurfaceY(bPos[0], bPos[2]);
					
					AnimalBase bAnimal = AnimalBase.Cast(GetGame().CreateObject(bearType, bPos, false, true));
					if (bAnimal) 
					{
						// ENTREGA O RECIBO AO WRECK
						wreck.AddAnimal(bAnimal);
						bearCount++;
					}
				}
			}
		}
		
		if (wolfCount > 0 || bearCount > 0) 
		{
			M4D_PlaneCrashLogger.TraceSpawn(wreck.GetEventID(), string.Format("Ameacas Animais instanciadas com sucesso: %1 Lobos, %2 Ursos.", wolfCount, bearCount));
		}
	}
}