// =====================================================================================
// M4D_PlaneCrashSpawner.c - VERSÃO DEFINITIVA (Blindagem contra HeliCrashes)
// Responsabilidade: Motor de geração do evento, validação de integridade e notificação.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE DE CONFIGURAÇÃO DE ASSETS (M4D_SpawnerAssetConfig)
	// ---------------------------------------------------------------------------------
	// Define o molde estrutural para cada objeto secundário do acidente (crateras, 
	// caixas militares). Armazena as coordenadas locais relativas ao avião, 
	// orientação e escala, permitindo uma construção matemática e modular da cena.
	// ---------------------------------------------------------------------------------
class M4D_SpawnerAssetConfig
{
	string AssetType;
	vector LocalOffset;
	vector Orientation;
	float Scale;
	bool AlignToTerrain;

	// ---------------------------------------------------------------------------------
	// CONSTRUTOR DA CLASSE (M4D_SpawnerAssetConfig)
	// ---------------------------------------------------------------------------------
	// Inicializa os parâmetros físicos que o Spawner usará para instanciar o objeto.
	// ---------------------------------------------------------------------------------
	void M4D_SpawnerAssetConfig(string type = "", vector offset = "0 0 0", vector orient = "0 0 0", float scale = 1.0, bool align = true)
	{
		AssetType = type;
		LocalOffset = offset;
		Orientation = orient;
		Scale = scale;
		AlignToTerrain = align;
	}
}

	// ---------------------------------------------------------------------------------
	// GERENCIADOR DE SPAWN (M4D_PlaneCrashSpawner)
	// ---------------------------------------------------------------------------------
	// A classe mestra responsável por orquestrar o evento. Ela localiza posições
	// válidas, instancia a fuselagem, avalia a integridade crítica do loot (Kill Switch),
	// aciona o Painel Vivo (WorldState) e dispara o broadcast de áudio e texto.
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashSpawner
{
	// ---------------------------------------------------------------------------------
	// DICIONÁRIO ESTÁTICO DE ASSETS (GetHardcodedAssets)
	// ---------------------------------------------------------------------------------
	// Retorna uma matriz contendo as coordenadas exatas e precisas de onde cada cratera
	// e caixa secundária deve nascer em relação ao centro do avião.
	// Sufixos falsos do editor (_3, _5) corrigidos para a classe base real da Engine.
	// ---------------------------------------------------------------------------------
	static ref array<ref M4D_SpawnerAssetConfig> GetHardcodedAssets()
	{
		ref array<ref M4D_SpawnerAssetConfig> assets = new array<ref M4D_SpawnerAssetConfig>();
		
		assets.Insert(new M4D_SpawnerAssetConfig("CraterLong", "-0.5940539836883545 -0.4000000059604645 10.352170944213868", "0.0 0.0 0.0", 1.0, true));
		assets.Insert(new M4D_SpawnerAssetConfig("CraterLong", "-0.48886001110076907 -0.4000000059604645 15.957640647888184", "0.35199999809265139 0.0 0.0", 1.0, true));
		assets.Insert(new M4D_SpawnerAssetConfig("M4DCrashStorage_MilitaryBox", "4.570658206939697 0.0 13.319387435913086", "12.247900009155274 0.0 0.0", 1.0, true));
		
		assets.Insert(new M4D_SpawnerAssetConfig("StaticObj_Misc_WoodenCrate", "-3.600000 0.0 12.989584", "0.0 0.0 0.0", 1.0, true));
		assets.Insert(new M4D_SpawnerAssetConfig("StaticObj_Misc_WoodenCrate", "-3.500000 0.0 15.989584", "138.00 0.0 0.0", 1.0, true));
		assets.Insert(new M4D_SpawnerAssetConfig("StaticObj_Misc_WoodenCrate", "-5.000000 0.0 18.489584", "127.60 0.0 0.0", 1.0, true));
		assets.Insert(new M4D_SpawnerAssetConfig("StaticObj_Misc_WoodenCrate", "4.060220 0.0 20.550507", "12.2479 0.0 0.0", 1.0, true));
		assets.Insert(new M4D_SpawnerAssetConfig("StaticObj_Misc_WoodenCrate", "4.525254 0.0 17.980886", "129.2479 0.0 0.0", 1.0, true));
		
		return assets;
	}

	// ---------------------------------------------------------------------------------
	// NOTIFICAÇÃO GLOBAL AOS JOGADORES (Broadcast Nativo)
	// ---------------------------------------------------------------------------------
	// Utiliza o sistema RPC puro da Engine do DayZ para enviar mensagens aos clientes.
	// Atualizado para varrer a lista de jogadores e despachar o pacote ponto-a-ponto,
	// evitando a filtragem e bloqueio de pacotes anônimos do motor de rede do DayZ.
	// ---------------------------------------------------------------------------------
	static void SendCrashNotification(string customMsg)
	{
		if (customMsg == "") 
		{
			return;
		}
		
		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		
		for (int i = 0; i < players.Count(); i++)
		{
			PlayerBase player = PlayerBase.Cast(players.Get(i));
			if (player)
			{
				if (player.GetIdentity())
				{
					GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>(customMsg), true, player.GetIdentity());
				}
			}
		}
	}

	// ---------------------------------------------------------------------------------
	// GERENCIADOR DE ABORTO E LIMPEZA (AbortAndCleanupEvent)
	// ---------------------------------------------------------------------------------
	// Centraliza a rotina de exclusão de um evento falho. Registra a string de motivo 
	// no Logger administrativo e aciona a deleção da fuselagem. O ato de deletar a 
	// fuselagem aciona automaticamente a EEDelete do Core.c, que executará a varredura 
	// de arrays pelo OwnerEventID, assegurando que não existam vazamentos de memória.
	// ---------------------------------------------------------------------------------
	static void AbortAndCleanupEvent(M4d_AirPlaneCrash wreck, string reason)
	{
		if (!wreck) 
		{
			return;
		}
		
		int eventId = wreck.GetEventID();
		M4D_PlaneCrashLogger.EventAborted(eventId, reason);
		GetGame().ObjectDelete(wreck);
	}

	// ---------------------------------------------------------------------------------
	// SORTEIO DE LOCAIS E TELEMETRIA (FindValidCrashSite)
	// ---------------------------------------------------------------------------------
	// Filtra a lista de locais do JSON para garantir que a coordenada sorteada não
	// possua jogadores próximos ou outros eventos na área (raio de 1000 metros).
	// ---------------------------------------------------------------------------------
	static vector FindValidCrashSite(out M4D_PlaneCrashCustomSite outChosenSite) 
	{
		ref M4D_PlaneCrashSites s_sites = M4D_PlaneCrashSites.Get();
		if (!s_sites) 
		{
			M4D_PlaneCrashLogger.Info("ABORTADO: O gerenciador de locais (PlaneCrashSites) nao esta carregado.");
			return vector.Zero;
		}
		
		outChosenSite = null;

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		
		if (s_sites.CustomCrashSites) 
		{
			if (s_sites.CustomCrashSites.Count() > 0) 
			{
				ref array<ref M4D_PlaneCrashCustomSite> availableSites = new array<ref M4D_PlaneCrashCustomSite>();
				
				int rejectedByPlayer = 0;
				int rejectedByOccupied = 0;

				for (int csIdx = 0; csIdx < s_sites.CustomCrashSites.Count(); csIdx++) 
				{
					M4D_PlaneCrashCustomSite cs = s_sites.CustomCrashSites.Get(csIdx);
					if (cs) 
					{
						vector testPos = Vector(cs.Position[0], GetGame().SurfaceY(cs.Position[0], cs.Position[2]), cs.Position[2]);
						
						bool playerTooClose = false;
						for (int pIdx = 0; pIdx < players.Count(); pIdx++) 
						{
							Man p = players.Get(pIdx);
							if (p) 
							{
								if (vector.Distance(testPos, p.GetPosition()) <= 1000.0) 
								{
									playerTooClose = true;
									break;
								}
							}
						}
						
						if (playerTooClose) 
						{
							rejectedByPlayer++;
							continue;
						}
						
						array<Object> nearby = new array<Object>();
						GetGame().GetObjectsAtPosition(testPos, 1000.0, nearby, null);
						
						bool occupied = false;
						for (int nIdx = 0; nIdx < nearby.Count(); nIdx++) 
						{
							Object obj = nearby.Get(nIdx);
							if (obj) 
							{
								string type = obj.GetType();
								
								// INJEÇÃO DA REGRA DE BLINDAGEM DE HELICRASHES NATIVOS
								// Se a área tiver nosso Avião, nosso Contentor ou os Helicrashes nativos, a área é abortada.
								if (type == "M4d_AirPlaneCrash" || type.Contains("M4D_WreckContainer") || type.Contains("Wreck_UH1Y") || type.Contains("Wreck_Mi8")) 
								{ 
									occupied = true; 
									break; 
								}
							}
						}
						
						if (!occupied) 
						{
							availableSites.Insert(cs);
						} 
						else 
						{
							rejectedByOccupied++;
						}
					}
				}
				
				if (availableSites.Count() > 0) 
				{
					outChosenSite = availableSites.GetRandomElement();
					if (outChosenSite) 
					{
						return Vector(outChosenSite.Position[0], GetGame().SurfaceY(outChosenSite.Position[0], outChosenSite.Position[2]), outChosenSite.Position[2]);
					}
				} 
				else 
				{
					M4D_PlaneCrashLogger.Info(string.Format("ABORTADO: Nenhum local valido encontrado. Total: %1 | Rej. Player: %2 | Rej. Ocupado/HeliCrash: %3", s_sites.CustomCrashSites.Count(), rejectedByPlayer, rejectedByOccupied));
				}
			} 
			else 
			{
				M4D_PlaneCrashLogger.Info("ABORTADO: A lista de 'CustomCrashSites' esta vazia no JSON.");
			}
		} 
		else 
		{
			M4D_PlaneCrashLogger.Info("ABORTADO: Configuracao 'CustomCrashSites' ausente ou corrompida.");
		}
		
		return vector.Zero;
	}

	// ---------------------------------------------------------------------------------
	// SELEÇÃO INTELIGENTE DE CONTENTORES (PickEnabledCrashContainerType)
	// ---------------------------------------------------------------------------------
	// Sorteia a cor do container baseando-se nos parâmetros ativos do administrador
	// no arquivo JSON, ou aplica um container preferido de forma forçada.
	// ---------------------------------------------------------------------------------
	static string PickEnabledCrashContainerType() 
	{
		ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
		if (!s) 
		{
			return "M4D_WreckContainerBlue";
		}

		string pref = s.PreferredContainer;
		
		if (pref != "" && pref != "Random") 
		{
			string prefLower = pref;
			prefLower.ToLower();

			if (prefLower == "red" && s.EnableContainerRed == 1) return "M4D_WreckContainerRed";
			if (prefLower == "blue" && s.EnableContainerBlue == 1) return "M4D_WreckContainerBlue";
			if (prefLower == "yellow" && s.EnableContainerYellow == 1) return "M4D_WreckContainerYellow";
			if (prefLower == "orange" && s.EnableContainerOrange == 1) return "M4D_WreckContainerOrange";
		}

		ref array<string> c = new array<string>();
		if (s.EnableContainerBlue == 1) c.Insert("M4D_WreckContainerBlue");
		if (s.EnableContainerRed == 1) c.Insert("M4D_WreckContainerRed");
		if (s.EnableContainerYellow == 1) c.Insert("M4D_WreckContainerYellow");
		if (s.EnableContainerOrange == 1) c.Insert("M4D_WreckContainerOrange");
		
		if (c.Count() > 0) 
		{
			return c.GetRandomElement();
		}
		
		return "M4D_WreckContainerBlue";
	}

	// ---------------------------------------------------------------------------------
	// MATEMÁTICA DE ORIENTAÇÃO (BuildWreckMatrix)
	// ---------------------------------------------------------------------------------
	// Calcula os vetores normais do terreno para garantir que o avião e todos os
	// objetos acompanhem corretamente os declives e inclinações do mapa.
	// ---------------------------------------------------------------------------------
	static void BuildWreckMatrix(vector pos, vector fwdOriginal, out vector mat[3]) 
	{
		vector normal = GetGame().SurfaceGetNormal(pos[0], pos[2]);
		vector fwd = vector.Direction(pos, pos + "1 0 0"); 
		if (fwdOriginal != "0 0 0") 
		{
			fwd = fwdOriginal;
		}

		fwd.Normalize();
		vector right = fwd * normal; 
		right.Normalize();
		fwd = normal * right; 
		fwd.Normalize();
		
		mat[0] = fwd; 
		mat[1] = normal; 
		mat[2] = right;
	}

	// ---------------------------------------------------------------------------------
	// INJEÇÃO DE ASSETS COM AUTO-HEALING (SpawnDetailAssets)
	// ---------------------------------------------------------------------------------
	// Distribui as crateras e caixas. Realiza a contagem tática e relata ao Logger.
	// ---------------------------------------------------------------------------------
	static void SpawnDetailAssets(M4d_AirPlaneCrash wreck, vector wreckMatrix[3], int tier, bool isReload)
	{
		ref array<ref M4D_SpawnerAssetConfig> assetsList = GetHardcodedAssets();

		vector dir = wreck.GetDirection(); 
		dir[1] = 0; 
		dir.Normalize();
		
		vector right = dir * "0 1 0"; 
		right.Normalize();
		
		vector wPos = wreck.GetPosition();

		if (isReload) 
		{
			M4D_PlaneCrashLogger.TraceSpawn(wreck.GetEventID(), string.Format("AUTO-HEAL INICIADO. Tier=%1", tier));
		}

		// Contadores para telemetria administrativa
		int countCrater = 0;
		int countMilitaryBox = 0;

		for (int x = 0; x < assetsList.Count(); x++) 
		{
			M4D_SpawnerAssetConfig cfg = assetsList.Get(x);
			
			vector bPos = wPos + (right * cfg.LocalOffset[0]) + (dir * cfg.LocalOffset[2]);
			float groundY = GetGame().SurfaceY(bPos[0], bPos[2]);
			float finalY;
			
			if (cfg.AlignToTerrain) 
			{
				if (cfg.LocalOffset[1] > 0) 
				{
					finalY = groundY + cfg.LocalOffset[1];
				}
				else 
				{
					finalY = groundY;
				}
			} 
			else 
			{
				finalY = wPos[1] + cfg.LocalOffset[1];
			}
			
			vector finalPos = Vector(bPos[0], finalY, bPos[2]);
			
			array<Object> checkObjs = new array<Object>(); 
			GetGame().GetObjectsAtPosition(finalPos, 2.5, checkObjs, null);
			bool exists = false;
			
			for (int y = 0; y < checkObjs.Count(); y++) 
			{
				Object o = checkObjs.Get(y);
				if (o)
				{
					if (o.GetType() == cfg.AssetType) 
					{ 
						exists = true; 
						break; 
					}
				}
			}
			
			if (exists) 
			{
				if (isReload) 
				{
					M4D_PlaneCrashLogger.TraceSpawn(wreck.GetEventID(), string.Format("AUTO-HEAL SKIP: %1 ja existe em %2", cfg.AssetType, finalPos.ToString()));
				}
				else
				{
					M4D_PlaneCrashLogger.TraceSpawn(wreck.GetEventID(), string.Format("ANTI-DUPE: Impedindo spawn duplicado de %1 na coordenada.", cfg.AssetType));
				}
				continue;
			}

			EntityAI asset = EntityAI.Cast(GetGame().CreateObject(cfg.AssetType, finalPos));
			if (asset) 
			{
				if (isReload) 
				{
					M4D_PlaneCrashLogger.TraceSpawn(wreck.GetEventID(), string.Format("AUTO-HEAL CREATE: %1 em %2", cfg.AssetType, finalPos.ToString()));
				}

				vector finalOri = wreck.GetOrientation() + cfg.Orientation;
				asset.SetPosition(finalPos);
				asset.SetOrientation(finalOri);
				
				if (Math.AbsFloat(cfg.Scale - 1.0) > 0.0001) 
				{
					asset.SetScale(cfg.Scale);
				}
				
				// Inclusão cirúrgica da WoodenCrate para acionar a colisão gravitacional da base da malha
				if (cfg.AssetType == "M4DCrashStorage_MilitaryBox" || cfg.AssetType == "StaticObj_Misc_WoodenCrate") 
				{
					asset.PlaceOnSurface();
				}

				M4d_AirPlaneCrash.M4D_UpdateNavmesh(asset);
				
				if (cfg.AssetType.Contains("M4DCrashStorage")) 
				{
					GetGame().GameScript.CallFunctionParams(asset, "SetOwnerEventID", null, new Param1<int>(wreck.GetEventID()));
				}
				
				wreck.AddDetailAsset(asset);

				// Soma para bloco narrativo de telemetria
				if (cfg.AssetType == "CraterLong") 
				{
					countCrater = countCrater + 1;
				} 
				else if (cfg.AssetType == "M4DCrashStorage_MilitaryBox") 
				{
					countMilitaryBox = countMilitaryBox + 1;
				} 
				
				if (!isReload) 
				{
					if (asset.GetType() == "M4DCrashStorage_MilitaryBox") 
					{
						M4D_PlaneCrashLootManager.FillMilitaryBoxLoot(asset, tier);
					}
				}
			} 
			else 
			{
				if (isReload) 
				{
					M4D_PlaneCrashLogger.TraceFailure(wreck.GetEventID(), string.Format("AUTO-HEAL FAIL: %1 nao criado em %2 (Falha silenciosa do Enforce)", cfg.AssetType, finalPos.ToString()));
				}
			}
		}

		if (!isReload)
		{
			string assetLog = string.Format("\n[ASSETS SPAWNED]\nEventID: %1\nCraterLong: %2\nM4DCrashStorage_MilitaryBox: %3", wreck.GetEventID(), countCrater, countMilitaryBox);
			M4D_PlaneCrashLogger.EventAsset(wreck.GetEventID(), assetLog);
		}
	}

	// ---------------------------------------------------------------------------------
	// ORQUESTRAÇÃO DE EVENTO E MALHA DE INTEGRIDADE (O KILL SWITCH)
	// ---------------------------------------------------------------------------------
	// A função mestra de montagem. Agora preenchida com os blocos de telemetria multiline.
	// ---------------------------------------------------------------------------------
	static void SpawnSite()
	{
		M4D_PlaneCrashCustomSite chosenSite;
		vector pos = FindValidCrashSite(chosenSite);
		
		if (pos == vector.Zero || !chosenSite) 
		{
			return;
		}

		int tier = chosenSite.Tier; 
		
		ref M4D_PlaneCrashSettings currentSettings = M4D_PlaneCrashSettings.Get();
		
		bool isGasSpawning = false;
		string gasStr = "Nao";
		string gasBool = "false";

		if (chosenSite.GasZone)
		{
			if (chosenSite.GasZone.Enabled == 1) 
			{
				isGasSpawning = true;
				gasStr = "Sim";
				gasBool = "true";
			}
		}

		M4d_AirPlaneCrash wreck = M4d_AirPlaneCrash.Cast(GetGame().CreateObject("M4d_AirPlaneCrash", pos));
		if (!wreck) 
		{
			return;
		}

		// ====== BLOCO TÁTICO: SITE SELECTED ======
		string notifMsgState = "vazia";
		if (chosenSite.NotificationMessage != "") 
		{ 
			notifMsgState = "preenchida"; 
		}

		string siteSelectedLog = string.Format("\n[EVENT SITE SELECTED]\nEventID: %1\nPosition: %2\nTier: %3\nGas: %4\nNotificationMessage: %5", wreck.GetEventID(), pos.ToString(), tier, gasStr, notifMsgState);
		M4D_PlaneCrashLogger.EventStart(wreck.GetEventID(), siteSelectedLog);
		// =========================================

		// ====== BLOCO TÁTICO: WRECK SPAWNED ======
		string wreckSpawnedLog = string.Format("\n[WRECK SPAWNED]\nEventID: %1\nClass: M4d_AirPlaneCrash\nPosition: %2\nTier: %3\nGas: %4", wreck.GetEventID(), pos.ToString(), tier, gasBool);
		M4D_PlaneCrashLogger.EventStart(wreck.GetEventID(), wreckSpawnedLog);
		// =========================================

		M4D_PlaneCrashWorldState.OpenEvent(wreck.GetEventID(), pos, tier, isGasSpawning);

		wreck.SetupEventState(tier, isGasSpawning);
		wreck.SetupThreatsData(chosenSite.ZombieTypes, chosenSite.WolfSpawn, chosenSite.BearSpawn);

		wreck.PlaceOnSurface();
		
		vector wPos = wreck.GetPosition();
		vector wMat[3]; 
		BuildWreckMatrix(wPos, "0 0 0", wMat);
		wreck.SetOrientation(Math3D.MatrixToAngles(wMat));
		wreck.SetPosition(wPos); 

		SpawnDetailAssets(wreck, wMat, tier, false);

		if (currentSettings)
		{
			if (currentSettings.EnableWreckPathgraphUpdate == 1) 
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(M4d_AirPlaneCrash.M4D_UpdateNavmesh, 500, false, wreck);
			}
		}

		string cType = PickEnabledCrashContainerType();
		EntityAI container = EntityAI.Cast(GetGame().CreateObject(cType, wPos + (wreck.GetDirection() * 25.0)));
		
		if (!container) 
		{
			AbortAndCleanupEvent(wreck, "Contentor Principal nao instanciado. Cleanup automatico executado.");
			return;
		}

		container.PlaceOnSurface();
		
		vector cPos = container.GetPosition();
		vector cNorm = GetGame().SurfaceGetNormal(cPos[0], cPos[2]);
		vector cFwd = vector.Direction(cPos, cPos + "1 0 0");
		
		vector cRight = cFwd * cNorm; 
		cRight.Normalize();
		
		cFwd = cNorm * cRight; 
		cFwd.Normalize();
		
		vector cMat[3]; 
		cMat[0] = cFwd; 
		cMat[1] = cNorm; 
		cMat[2] = cRight;
		
		container.SetOrientation(Math3D.MatrixToAngles(cMat));
		container.SetPosition(cPos);

		// ====== BLOCO TÁTICO: CONTAINER SPAWNED ======
		string containerSpawnedLog = string.Format("\n[CONTAINER SPAWNED]\nEventID: %1\nContainerType: %2\nPosition: %3\nInventoryNative: locked", wreck.GetEventID(), cType, cPos.ToString());
		M4D_PlaneCrashLogger.EventContainer(wreck.GetEventID(), containerSpawnedLog);
		// =============================================

		GetGame().GameScript.CallFunctionParams(container, "SetOwnerEventID", null, new Param1<int>(wreck.GetEventID()));
		wreck.SetMainContainer(container);

		if (currentSettings)
		{
			if (currentSettings.EnableContainerPathgraphUpdate == 1) 
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(M4d_AirPlaneCrash.M4D_UpdateNavmesh, 500, false, container);
			}
		}

		vector spawnPos = container.ModelToWorld("-0.1 -0.4 0"); 
		M4D_CrashRewardChest chest = M4D_CrashRewardChest.Cast(GetGame().CreateObject("M4D_CrashRewardChest", spawnPos));
		
		if (!chest) 
		{
			AbortAndCleanupEvent(wreck, "M4D_CrashRewardChest nao instanciado. Cleanup automatico executado.");
			return;
		}

		chest.SetOrientation(container.GetOrientation());
		chest.SetOwnerEventID(wreck.GetEventID());
		
		chest.SetSystemSpawningMode(true);
		int itemsSpawned = M4D_PlaneCrashLootManager.FillContainerWithRandomLoot(chest, tier);
		chest.SetSystemSpawningMode(false);
		
		if (itemsSpawned <= 0)
		{
			AbortAndCleanupEvent(wreck, "LootResult: ItemsSpawned=0. Nenhuma recompensa gerada. Cleanup automatico executado.");
			return;
		}

		// ====== BLOCO TÁTICO: REWARD CHEST SPAWNED ======
		string chestSpawnedLog = string.Format("\n[REWARD CHEST SPAWNED]\nEventID: %1\nChestClass: M4D_CrashRewardChest\nState: locked\nLootGenerated: yes", wreck.GetEventID());
		M4D_PlaneCrashLogger.EventContainer(wreck.GetEventID(), chestSpawnedLog);
		// ================================================

		chest.SetInventoryVisibility(false);
		M4D_PlaneCrashWorldState.MarkChestSpawned(wreck.GetEventID());

		M4D_PlaneCrashWorldState.UpdateCounts(wreck.GetEventID(), wreck.GetDetailAssetsCount(), 0, 0, 0);

		string key = "ShippingContainerKeys_Blue";
		if (cType.Contains("Red")) 
		{
			key = "ShippingContainerKeys_Red";
		}
		else if (cType.Contains("Yellow")) 
		{
			key = "ShippingContainerKeys_Yellow";
		}
		else if (cType.Contains("Orange")) 
		{
			key = "ShippingContainerKeys_Orange";
		}
		
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(M4D_PlaneCrashThreats.SpawnThreatsInstance, 1000, false, wreck, isGasSpawning);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(M4D_PlaneCrashThreats.M4D_SpawnZombiesWithKey, 3000, false, wreck, pos, key);
		
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(M4D_PlaneCrashSpawner.ConsolidateEventState, 5000, false, wreck);

		// ====== BLOCO TÁTICO: NOTIFICATION SENT/SKIPPED ======
		if (currentSettings)
		{
			if (currentSettings.EnableCrashNotification == 1) 
			{
				string customMsg = chosenSite.NotificationMessage; 
				if (customMsg != "")
				{
					SendCrashNotification(customMsg);
					string notifSentLog = string.Format("\n[NOTIFICATION SENT]\nEventID: %1\nEnabled: true\nMessage: %2\nMode: DayZ native broadcast", wreck.GetEventID(), customMsg);
					M4D_PlaneCrashLogger.EventStart(wreck.GetEventID(), notifSentLog);
				}
				else
				{
					string notifSkippedLog = string.Format("\n[NOTIFICATION SKIPPED]\nEventID: %1\nReason: NotificationMessage empty", wreck.GetEventID());
					M4D_PlaneCrashLogger.EventStart(wreck.GetEventID(), notifSkippedLog);
				}
			}
			else
			{
				string notifSkippedLog2 = string.Format("\n[NOTIFICATION SKIPPED]\nEventID: %1\nReason: Notifications disabled in settings", wreck.GetEventID());
				M4D_PlaneCrashLogger.EventStart(wreck.GetEventID(), notifSkippedLog2);
			}
		}
		// =====================================================

		// ====== BLOCO TÁTICO: CRASH SOUND DISPATCHED ======
		int playersReached = 0;
		const int M4D_RPC_CRASH_SOUND = 4857392; 
		array<Man> playersForSound = new array<Man>(); 
		GetGame().GetPlayers(playersForSound);
		
		for (int pIdx = 0; pIdx < playersForSound.Count(); pIdx++) 
		{
			PlayerBase playerToHear = PlayerBase.Cast(playersForSound.Get(pIdx));
			if (playerToHear)
			{
				if (playerToHear.GetIdentity()) 
				{
					float distance = vector.Distance(playerToHear.GetPosition(), wPos);
					if (distance <= 3500.0) 
					{
						playersReached = playersReached + 1;
						float speedOfSound = 343.0;
						float delaySeconds = distance / speedOfSound;
						int delayMilliseconds = Math.Round(delaySeconds * 1000);
						GetGame().RPCSingleParam(playerToHear, M4D_RPC_CRASH_SOUND, new Param2<vector, int>(wPos, delayMilliseconds), true, playerToHear.GetIdentity());
					}
				}
			}
		}

		string soundDispatchedLog = string.Format("\n[CRASH SOUND DISPATCHED]\nEventID: %1\nRadius: 3500m\nPlayersReached: %2", wreck.GetEventID(), playersReached);
		M4D_PlaneCrashLogger.EventStart(wreck.GetEventID(), soundDispatchedLog);
		// ==================================================
	}

	// ---------------------------------------------------------------------------------
	// CONSOLIDAÇÃO DE WORLDSTATE (ConsolidateEventState)
	// ---------------------------------------------------------------------------------
	// Executado em delay. O Spawner recolhe do avião as contagens finais de zombies 
	// e ameaças instanciadas, registrando-os no Painel Vivo do servidor.
	// ---------------------------------------------------------------------------------
	static void ConsolidateEventState(M4d_AirPlaneCrash wreck) 
	{
		if (!wreck || !GetGame() || !GetGame().IsServer()) 
		{
			return;
		}
		
		M4D_PlaneCrashWorldState.UpdateCounts(wreck.GetEventID(), wreck.GetDetailAssetsCount(), wreck.GetZombiesCount(), wreck.GetAnimalsCount(), wreck.GetGasAreasCount());
		M4D_PlaneCrashLogger.TraceFunction(wreck.GetEventID(), "Estado final do evento consolidado no WorldState com sucesso.");
	}
}