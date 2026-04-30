// =====================================================================================
// M4D_PlaneCrashLootManager.c - VERSÃO DEFINITIVA (Unificação Militar e Recibos)
// Responsabilidade: Gestão e distribuição de loot baseado em probabilidades e Tiers,
// agora centralizado na MilitaryBox e retornando a integridade para validação.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE GERENCIADORA DE LOOT (M4D_PlaneCrashLootManager)
	// ---------------------------------------------------------------------------------
	// Centraliza toda a matemática de probabilidade e instanciação de itens.
	// Atua como um operário que preenche as caixas e retorna um "recibo" numérico
	// informando quantos itens foram criados com sucesso para o Spawner avaliar.
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashLootManager
{
	// ---------------------------------------------------------------------------------
	// CONTENTOR PRINCIPAL (Preenchimento do Baú Dinâmico)
	// ---------------------------------------------------------------------------------
	// Esta função recebe o baú físico recém-instanciado (M4D_CrashRewardChest) 
	// e define qual tabela de loot será usada com base no Tier do local. 
	// Retorna o total de itens primários gerados com sucesso.
	// ---------------------------------------------------------------------------------
	static int FillContainerWithRandomLoot(EntityAI container, int tier) 
	{
		if (!container) return 0;
		if (!container.GetInventory()) return 0;
		
		ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
		ref M4D_PlaneCrashLoot l = M4D_PlaneCrashLoot.Get();
		
		if (!s) return 0;
		if (!l) return 0;
		
		ref array<ref M4D_PlaneCrashLootEntry> activeLoot = l.DefaultLootItems;
		
		if (s.EnableDefaultLootItems == 0) 
		{
			if (tier == 1) 
			{
				activeLoot = l.Tier1_MainLoot;
			}
			else if (tier == 2) 
			{
				activeLoot = l.Tier2_MainLoot;
			}
			else if (tier == 3) 
			{
				activeLoot = l.Tier3_MainLoot;
			}
		}

		return ProcessLootSpawning(container, activeLoot, s.MinLootItems, s.MaxLootItems);
	}

	// ---------------------------------------------------------------------------------
	// CAIXAS MILITARES DE SUPRIMENTOS (Preenchimento Standalone Unificado)
	// ---------------------------------------------------------------------------------
	// Recebe a caixa militar unificada, define a tabela correta do JSON e invoca o
	// motor matemático de spawn, retornando a contagem de itens ao final.
	// ---------------------------------------------------------------------------------
	static int FillMilitaryBoxLoot(EntityAI box, int tier) 
	{
		if (!box) return 0;
		
		ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
		ref M4D_PlaneCrashLoot l = M4D_PlaneCrashLoot.Get();
		
		if (!s) return 0;
		if (!l) return 0;

		ref array<ref M4D_PlaneCrashLootEntry> activeLoot = l.DefaultMilitaryBoxLoot;
		
		if (s.EnableDefaultMilitaryBoxLoot == 0) 
		{
			if (tier == 1) 
			{
				activeLoot = l.Tier1_MilitaryBoxLoot;
			}
			else if (tier == 2) 
			{
				activeLoot = l.Tier2_MilitaryBoxLoot;
			}
			else if (tier == 3) 
			{
				activeLoot = l.Tier3_MilitaryBoxLoot;
			}
		}

		return ProcessLootSpawning(box, activeLoot, s.MinMilitaryBoxItems, s.MaxMilitaryBoxItems);
	}

	// ---------------------------------------------------------------------------------
	// MOTOR MATEMÁTICO DE SPAWN (Core do Loot com Bypass Integrado Universal)
	// ---------------------------------------------------------------------------------
	// Instancia fisicamente os itens nos inventários. Aplica a probabilidade (Chance),
	// manipula anexos e carga interna. No final, soma todos os itens primários
	// instanciados e devolve esse número ("recibo") para a função chamadora.
	// ---------------------------------------------------------------------------------
	protected static int ProcessLootSpawning(EntityAI target, array<ref M4D_PlaneCrashLootEntry> lootPool, int minItems, int maxItems)
	{
		if (!target) return 0;
		if (!target.GetInventory()) return 0;
		if (!lootPool) return 0;
		if (lootPool.Count() == 0) return 0;
		
		int spawnedItemsCount = 0;

		// 1. LIGA O BYPASS: Avaliação estrita sem operadores ternários para proteger a memória
		M4D_CrashRewardChest customBox = M4D_CrashRewardChest.Cast(target);
		if (customBox) 
		{
			customBox.SetSystemSpawningMode(true);
		}

		M4DCrashStorage storageBox = M4DCrashStorage.Cast(target);
		if (storageBox) 
		{
			storageBox.SetSystemSpawningMode(true);
		}

		ref array<ref M4D_PlaneCrashLootEntry> tempPool = new array<ref M4D_PlaneCrashLootEntry>();
		for (int a = 0; a < lootPool.Count(); a++) 
		{ 
			tempPool.Insert(lootPool.Get(a)); 
		}
		
		// Embaralhamento (Shuffle)
		for (int i = 0; i < tempPool.Count(); i++) 
		{
			int j = Math.RandomInt(i, tempPool.Count());
			M4D_PlaneCrashLootEntry t = tempPool[i]; 
			tempPool[i] = tempPool[j]; 
			tempPool[j] = t;
		}

		int spawnCount = Math.RandomIntInclusive(minItems, maxItems);
		array<ref M4D_PlaneCrashLootEntry> selected = new array<ref M4D_PlaneCrashLootEntry>();
		
		for (int b = 0; b < tempPool.Count(); b++) 
		{
			M4D_PlaneCrashLootEntry ent = tempPool.Get(b);
			if (selected.Count() >= spawnCount) 
			{
				break;
			}
			
			if (Math.RandomFloat(0.0, 1.0) <= ent.Chance) 
			{ 
				selected.Insert(ent); 
			}
		}

		bool forcePristine = false;
		ref M4D_PlaneCrashSettings st = M4D_PlaneCrashSettings.Get();
		if (st)
		{
			if (st.PristineLoot == 1) 
			{
				forcePristine = true;
			}
		}
		
		for (int c = 0; c < selected.Count(); c++) 
		{
			M4D_PlaneCrashLootEntry finalLoot = selected.Get(c);
			EntityAI item = target.GetInventory().CreateInInventory(finalLoot.Item);
			
			if (!item) 
			{
				continue;
			}
			
			spawnedItemsCount++;
			
			float hp = Math.RandomFloat(0.4, 1.0);
			if (forcePristine) 
			{
				hp = 1.0;
			}
			item.SetHealth01("", "", hp);

			if (item.GetInventory()) 
			{
				for (int d = 0; d < finalLoot.Attachments.Count(); d++) 
				{
					string attName = finalLoot.Attachments.Get(d);
					EntityAI att = item.GetInventory().CreateAttachment(attName);
					
					if (!att) 
					{
						att = item.GetInventory().CreateInInventory(attName);
					}
					
					if (att) 
					{
						att.SetHealth01("", "", hp);
					}
				}
				for (int e = 0; e < finalLoot.CargoItems.Count(); e++) 
				{
					string cargoName = finalLoot.CargoItems.Get(e);
					EntityAI cargoItem = item.GetInventory().CreateInInventory(cargoName);
					
					if (!cargoItem) 
					{
						cargoItem = item.GetInventory().CreateEntityInCargo(cargoName);
					}
					
					if (cargoItem) 
					{
						cargoItem.SetHealth01("", "", hp);
					}
				}
			}
		}

		// 2. DESLIGA O BYPASS: Tranca as caixas novamente contra os jogadores
		if (customBox) 
		{
			customBox.SetSystemSpawningMode(false);
		}

		if (storageBox) 
		{
			storageBox.SetSystemSpawningMode(false);
		}
		
		return spawnedItemsCount;
	}
}