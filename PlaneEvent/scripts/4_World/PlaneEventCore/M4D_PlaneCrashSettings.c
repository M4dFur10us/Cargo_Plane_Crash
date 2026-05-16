// =====================================================================================
// M4D_PlaneCrashSettings.c - VERSÃO FINAL CONSOLIDADA (Unificação Militar JSON + Consumo Chave)
// Responsabilidade: Gestão centralizada de configurações, locais e tabelas de loot.
// =====================================================================================

// =====================================================================================
// CLASSES DE ESTRUTURA DE DADOS (JSON)
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE DE ESTRUTURA DE DADOS ANIMAL (PlaneCrashAnimalSpawn)
	// ---------------------------------------------------------------------------------
	// Molde para armazenar as configurações de spawn de lobos e ursos associados 
	// ao evento, permitindo serialização e desserialização via JsonFileLoader.
	// ---------------------------------------------------------------------------------
class PlaneCrashAnimalSpawn
{
	int Enabled;
	int Count;
	float Radius;
	ref array<string> Types;

	void PlaneCrashAnimalSpawn()
	{
		Enabled = 0; 
		Count = 0; 
		Radius = 0.0;
		Types = new array<string>();
	}
}

	// ---------------------------------------------------------------------------------
	// CLASSE DE ESTRUTURA DE DADOS DE GÁS (PlaneCrashGasZone)
	// ---------------------------------------------------------------------------------
	// Molde simples para armazenar a flag que define se a zona de queda 
	// terá ou não uma área contaminada de gás estática.
	// ---------------------------------------------------------------------------------
class PlaneCrashGasZone
{
	int Enabled;

	void PlaneCrashGasZone()
	{
		Enabled = 0;
	}
}

	// ---------------------------------------------------------------------------------
	// CLASSE DE DEFINIÇÃO DE LOCAL (M4D_PlaneCrashCustomSite)
	// ---------------------------------------------------------------------------------
	// Molde que define uma coordenada customizada de queda, englobando a posição,
	// mensagem de notificação, tier do local e os pacotes de ameaças específicos.
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashCustomSite
{
	vector Position;
	string NotificationMessage;
	int Tier;

	ref array<string>         ZombieTypes;
	ref PlaneCrashAnimalSpawn WolfSpawn;
	ref PlaneCrashAnimalSpawn BearSpawn;
	ref PlaneCrashGasZone     GasZone;
}

	// ---------------------------------------------------------------------------------
	// CLASSE DE ENTRADA DE LOOT (M4D_PlaneCrashLootEntry)
	// ---------------------------------------------------------------------------------
	// Define um item específico de loot, a sua probabilidade de spawn (Chance) 
	// e listas de anexos e itens que devem ser injetados no seu inventário interno.
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashLootEntry
{
	string Item;
	float  Chance; 
	
	ref array<string> Attachments;
	ref array<string> CargoItems;

	void M4D_PlaneCrashLootEntry(string item = "", float chance = 1.0)
	{
		Item = item;
		Chance = chance;
		Attachments = new array<string>();
		CargoItems = new array<string>();
	}
}

// =====================================================================================
// GERENCIADOR 1: LOCAIS (PlaneCrashSites.json)
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// GERENCIADOR DE LOCAIS (M4D_PlaneCrashSites)
	// ---------------------------------------------------------------------------------
	// Singleton responsável por gerenciar e manter em RAM a lista de todos os locais 
	// de queda possíveis. Carrega os dados do disco rígido ou recria o arquivo padrão.
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashSites
{
	static const string SITES_PATH = "$profile:M4D_AirPlaneCrash/PlaneCrashSites.json";
	ref array<ref M4D_PlaneCrashCustomSite> CustomCrashSites;

	private static ref M4D_PlaneCrashSites s_Instance;

	static M4D_PlaneCrashSites Get()
	{
		if (!s_Instance) 
		{
			s_Instance = new M4D_PlaneCrashSites();
			s_Instance.Load();
		}
		return s_Instance;
	}

	void Load()
	{
		if (FileExist(SITES_PATH)) 
		{
			string errorMessage;
			if (!JsonFileLoader<M4D_PlaneCrashSites>.LoadFile(SITES_PATH, this, errorMessage)) 
			{
				Print("[M4D_PlaneCrash] ERROR: Falha ao ler PlaneCrashSites.json! Restaurando backup nativo...");
				BuildDefault();
				Save();
			}
		} 
		else 
		{
			BuildDefault();
			Save();
		}
	}

	void Save() 
	{ 
		JsonFileLoader<M4D_PlaneCrashSites>.JsonSaveFile(SITES_PATH, this); 
	}

	void AddSite(vector pos, string msg, int tier)
	{
		M4D_PlaneCrashCustomSite s = new M4D_PlaneCrashCustomSite();
		s.Position = pos;
		s.NotificationMessage = msg;
		s.Tier = tier;
		
		s.ZombieTypes = new array<string>();
		s.ZombieTypes.InsertAll({
			"ZmbM_NBC_Grey",
			"ZmbM_PatrolNormal_Autumn",
			"ZmbM_PatrolNormal_Flat",
			"ZmbM_PatrolNormal_PautRev",
			"ZmbM_PatrolNormal_Summer",
			"ZmbM_SoldierNormal",
			"ZmbM_usSoldier_Heavy_Woodland",
			"ZmbM_usSoldier_Officer_Desert",
			"ZmbM_usSoldier_normal_Desert",
			"ZmbM_usSoldier_normal_Woodland"
		});
		
		s.WolfSpawn = new PlaneCrashAnimalSpawn();
		s.WolfSpawn.Enabled = 0;
		s.WolfSpawn.Count = 0;
		s.WolfSpawn.Radius = 0.0;
		s.WolfSpawn.Types.Insert("Animal_CanisLupus_Grey");
		s.WolfSpawn.Types.Insert("Animal_CanisLupus_White");
		
		s.BearSpawn = new PlaneCrashAnimalSpawn();
		s.BearSpawn.Enabled = 0;
		s.BearSpawn.Count = 0;
		s.BearSpawn.Radius = 0.0;
		s.BearSpawn.Types.Insert("Animal_UrsusArctos");
		
		s.GasZone = new PlaneCrashGasZone();
		s.GasZone.Enabled = 0;
		
		CustomCrashSites.Insert(s);
	}

	void BuildDefault()
	{
		CustomCrashSites = new array<ref M4D_PlaneCrashCustomSite>();
		
		AddSite(Vector(4748.660156, 0.0, 9732.080078), "Um avião foi abatido próximo a Airfield!", 3);
		AddSite(Vector(1269.11, 0.0, 5860.32), "Um avião foi abatido próximo a Plotina Tishina!", 3);
		AddSite(Vector(2731.03, 0.0, 3215.99), "Um avião foi abatido próximo a Maryanka!", 3);
		AddSite(Vector(5750.89, 0.0, 7614.38), "Um avião foi abatido próximo a Stary Sobor!", 3);
		AddSite(Vector(4600.42, 0.0, 12718.83), "Um avião foi abatido próximo a Zaprudnoye!", 3);
		AddSite(Vector(12312.50, 0.0, 12438.87), "Um avião foi abatido próximo a Airfield Krasno!", 1);
		AddSite(Vector(13999.56, 0.0, 14959.98), "Um avião foi abatido próximo a Belaya Polyana!", 1);
		AddSite(Vector(11399.70, 0.0, 7258.60), "Um avião foi abatido próximo a Youth Pioneer!", 1);
		AddSite(Vector(9141.54, 0.0, 3512.16), "Um avião foi abatido próximo a Pusta!", 1);
		AddSite(Vector(5127.78, 0.0, 2314.06), "Um avião foi abatido próximo a Airfield Balota!", 1);
		AddSite(Vector(9200.35, 0.0, 11807.50), "Um avião foi abatido próximo a Novy Lug!", 2);
		AddSite(Vector(4152.77, 0.0, 4807.96), "Um avião foi abatido próximo a Kozlovka!", 2);
		AddSite(Vector(4499.81, 0.0, 5960.86), "Um avião foi abatido próximo a Green Mountain!", 2);
		AddSite(Vector(6386.63, 0.0, 6712.77), "Um avião foi abatido próximo a Pop Ivan!", 2);
		AddSite(Vector(7753.39, 0.0, 8821.72), "Um avião foi abatido próximo a Gnomovzamok!", 2);
	}
}

// =====================================================================================
// GERENCIADOR 2: LOOT (PlaneCrashLoot.json)
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// GERENCIADOR DE LOOT (M4D_PlaneCrashLoot)
	// ---------------------------------------------------------------------------------
	// Singleton que armazena todas as tabelas de loot, divididas categoricamente 
	// em Tiers e tipos de contêiner. Carrega os arrays a partir do disco rígido.
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashLoot
{
	static const string LOOT_PATH = "$profile:M4D_AirPlaneCrash/PlaneCrashLoot.json";

	ref array<ref M4D_PlaneCrashLootEntry> Tier1_MainLoot;
	ref array<ref M4D_PlaneCrashLootEntry> Tier2_MainLoot;
	ref array<ref M4D_PlaneCrashLootEntry> Tier3_MainLoot;
	
	// Unificação das matrizes de Loot da Caixa Militar
	ref array<ref M4D_PlaneCrashLootEntry> Tier1_MilitaryBoxLoot;
	ref array<ref M4D_PlaneCrashLootEntry> Tier2_MilitaryBoxLoot;
	ref array<ref M4D_PlaneCrashLootEntry> Tier3_MilitaryBoxLoot;
	
	ref array<ref M4D_PlaneCrashLootEntry> DefaultLootItems;
	ref array<ref M4D_PlaneCrashLootEntry> DefaultMilitaryBoxLoot;

	private static ref M4D_PlaneCrashLoot s_Instance;

	static M4D_PlaneCrashLoot Get()
	{
		if (!s_Instance) 
		{
			s_Instance = new M4D_PlaneCrashLoot();
			s_Instance.Load();
		}
		return s_Instance;
	}

	void Load()
	{
		if (FileExist(LOOT_PATH)) 
		{
			string errorMessage;
			if (!JsonFileLoader<M4D_PlaneCrashLoot>.LoadFile(LOOT_PATH, this, errorMessage)) 
			{
				Print("[M4D_PlaneCrash] ERROR: Falha ao ler PlaneCrashLoot.json! Restaurando backup nativo...");
				BuildDefault();
				Save();
			}
		} 
		else 
		{
			BuildDefault();
			Save();
		}
	}

	void Save() 
	{ 
		JsonFileLoader<M4D_PlaneCrashLoot>.JsonSaveFile(LOOT_PATH, this); 
	}

	void BuildDefault()
	{
		Tier1_MainLoot = new array<ref M4D_PlaneCrashLootEntry>();
		Tier2_MainLoot = new array<ref M4D_PlaneCrashLootEntry>();
		Tier3_MainLoot = new array<ref M4D_PlaneCrashLootEntry>();
		
		Tier1_MilitaryBoxLoot = new array<ref M4D_PlaneCrashLootEntry>();
		Tier2_MilitaryBoxLoot = new array<ref M4D_PlaneCrashLootEntry>();
		Tier3_MilitaryBoxLoot = new array<ref M4D_PlaneCrashLootEntry>();
		
		DefaultLootItems = new array<ref M4D_PlaneCrashLootEntry>();
		DefaultMilitaryBoxLoot = new array<ref M4D_PlaneCrashLootEntry>();

		M4D_PlaneCrashLootEntry e;

		// --- TIER 1 MAIN ---
		e = new M4D_PlaneCrashLootEntry("M4A1", 1.0); e.Attachments.InsertAll({"M4_OEBttstck", "M4_PlasticHndgrd", "Mag_CMAG_30Rnd"}); Tier1_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("SVD", 1.0); e.Attachments.InsertAll({"Mag_SVD_10Rnd", "PSO11Optic"}); Tier1_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("PlateCarrierVest_Black", 1.0); e.Attachments.InsertAll({"PlateCarrierHolster_Black", "PlateCarrierPouches_Black"}); Tier1_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Plastic_Explosive", 1.0); Tier1_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Mag_AKM_Drum75Rnd", 1.0); Tier1_MainLoot.Insert(e);

		// --- TIER 2 MAIN ---
		e = new M4D_PlaneCrashLootEntry("M14", 1.0); e.Attachments.InsertAll({"Mag_M14_20Rnd", "ACOGOptic_6x"}); Tier2_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("SVD", 1.0); e.Attachments.InsertAll({"Mag_SVD_10Rnd", "PSO11Optic"}); Tier2_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("PlateCarrierVest_Black", 1.0); e.Attachments.InsertAll({"PlateCarrierHolster_Black", "PlateCarrierPouches_Black"}); Tier2_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Grenade_ChemGas", 1.0); Tier2_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Ammo_40mm_Explosive", 1.0); Tier2_MainLoot.Insert(e);

		// --- TIER 3 MAIN ---
		e = new M4D_PlaneCrashLootEntry("SVD", 1.0); e.Attachments.InsertAll({"Mag_SVD_10Rnd", "PSO11Optic"}); Tier3_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("FAL", 1.0); e.Attachments.InsertAll({"Fal_FoldingBttstck", "ACOGOptic", "Mag_FAL_20Rnd"}); Tier3_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Ammo_40mm_Chemgas", 1.0); Tier3_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("M79", 1.0); Tier3_MainLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Ammo_40mm_Explosive", 1.0); Tier3_MainLoot.Insert(e);

		// --- TIER 1 MILITARY BOX ---
		e = new M4D_PlaneCrashLootEntry("AKS74U", 1.0); e.Attachments.Insert("AKS74U_Bttstck"); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("SKS", 1.0); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Glock19", 1.0); e.Attachments.Insert("Mag_Glock_15Rnd"); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("MP5K", 1.0); e.Attachments.InsertAll({"MP5_PlasticHndgrd", "Mag_MP5_15Rnd"}); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("NailBox", 1.0); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Pliers", 1.0); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Pickaxe", 1.0); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Whetstone", 1.0); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("MetalPlate", 1.0); Tier1_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("MetalWire", 1.0); Tier1_MilitaryBoxLoot.Insert(e);

		// --- TIER 2 MILITARY BOX (Correção de Tendas) ---
		e = new M4D_PlaneCrashLootEntry("AK74", 1.0); e.Attachments.InsertAll({"AK74_WoodBttstck", "AK74_Hndgrd", "Mag_AK74_30Rnd"}); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Mosin9130", 1.0); e.Attachments.Insert("PUScopeOptic"); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("FNX45", 1.0); e.Attachments.Insert("Mag_FNX45_15Rnd"); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("UMP45", 1.0); e.Attachments.Insert("Mag_UMP_25Rnd"); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("LeatherSewingKit", 1.0); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("WeaponCleaningKit", 1.0); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("EpoxyPutty", 1.0); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("LargeTent", 1.0); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("MediumTent", 1.0); Tier2_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("CarTent", 1.0); Tier2_MilitaryBoxLoot.Insert(e);

		// --- TIER 3 MILITARY BOX (Correção de Claymore) ---
		e = new M4D_PlaneCrashLootEntry("FAMAS", 1.0); e.Attachments.Insert("Mag_FAMAS_25Rnd"); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("M16A2", 1.0); e.Attachments.Insert("Mag_CMAG_20Rnd"); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Winchester70", 1.0); e.Attachments.Insert("HuntingOptic"); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Saiga", 1.0); e.Attachments.InsertAll({"Saiga_Bttstck", "Mag_Saiga_8Rnd"}); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("PlateCarrierVest_Camo", 1.0); e.Attachments.InsertAll({"PlateCarrierHolster_Camo", "PlateCarrierPouches_Camo"}); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("NVGHeadstrap", 1.0); e.Attachments.Insert("NVGoggles"); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("GorkaHelmet_Black", 1.0); e.Attachments.Insert("GorkaHelmetVisor"); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("M67Grenade", 1.0); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("RGD5Grenade", 1.0); Tier3_MilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("ClaymoreMine", 1.0); Tier3_MilitaryBoxLoot.Insert(e);

		// --- DEFAULTS (BAÚ PRINCIPAL) ---
		e = new M4D_PlaneCrashLootEntry("M4A1", 1.0); e.Attachments.InsertAll({"M4_OEBttstck", "M4_PlasticHndgrd", "Mag_CMAG_30Rnd"}); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("M14", 1.0); e.Attachments.InsertAll({"Mag_M14_20Rnd", "ACOGOptic_6x"}); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("SVD", 1.0); e.Attachments.InsertAll({"Mag_SVD_10Rnd", "PSO11Optic"}); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("FAL", 1.0); e.Attachments.InsertAll({"Fal_FoldingBttstck", "ACOGOptic", "Mag_FAL_20Rnd"}); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("PlateCarrierVest_Black", 1.0); e.Attachments.InsertAll({"PlateCarrierHolster_Black", "PlateCarrierPouches_Black"}); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Plastic_Explosive", 1.0); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Grenade_ChemGas", 1.0); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("M79", 1.0); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Mag_AKM_Drum75Rnd", 1.0); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Ammo_40mm_Explosive", 1.0); DefaultLootItems.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Ammo_40mm_Chemgas", 1.0); DefaultLootItems.Insert(e);

		// --- DEFAULTS (MILITARY BOX UNIFICADA CORRIGIDA) ---
		e = new M4D_PlaneCrashLootEntry("AKS74U", 1.0); e.Attachments.Insert("AKS74U_Bttstck"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("SKS", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Glock19", 1.0); e.Attachments.Insert("Mag_Glock_15Rnd"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("MP5K", 1.0); e.Attachments.InsertAll({"MP5_PlasticHndgrd", "Mag_MP5_15Rnd"}); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("NailBox", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Pliers", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Pickaxe", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Whetstone", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("MetalPlate", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("MetalWire", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("AK74", 1.0); e.Attachments.InsertAll({"AK74_WoodBttstck", "AK74_Hndgrd", "Mag_AK74_30Rnd"}); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Mosin9130", 1.0); e.Attachments.Insert("PUScopeOptic"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("FNX45", 1.0); e.Attachments.Insert("Mag_FNX45_15Rnd"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("UMP45", 1.0); e.Attachments.Insert("Mag_UMP_25Rnd"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("LeatherSewingKit", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("WeaponCleaningKit", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("EpoxyPutty", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("LargeTent", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("MediumTent", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("CarTent", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("FAMAS", 1.0); e.Attachments.Insert("Mag_FAMAS_25Rnd"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("M16A2", 1.0); e.Attachments.Insert("Mag_CMAG_20Rnd"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Winchester70", 1.0); e.Attachments.Insert("HuntingOptic"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("Saiga", 1.0); e.Attachments.InsertAll({"Saiga_Bttstck", "Mag_Saiga_8Rnd"}); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("PlateCarrierVest_Camo", 1.0); e.Attachments.InsertAll({"PlateCarrierHolster_Camo", "PlateCarrierPouches_Camo"}); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("NVGHeadstrap", 1.0); e.Attachments.Insert("NVGoggles"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("GorkaHelmet_Black", 1.0); e.Attachments.Insert("GorkaHelmetVisor"); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("M67Grenade", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("RGD5Grenade", 1.0); DefaultMilitaryBoxLoot.Insert(e);
		e = new M4D_PlaneCrashLootEntry("ClaymoreMine", 1.0); DefaultMilitaryBoxLoot.Insert(e);
	}
}

// =====================================================================================
// GERENCIADOR 3: CONFIGURAÇÕES GERAIS (PlaneCrashSettings.json)
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DE CONFIGURAÇÕES (M4D_PlaneCrashSettings)
	// ---------------------------------------------------------------------------------
	// Entidade que armazena todas as variáveis globais de controle do mod e 
	// agora orquestra o sistema expandido e duplo de logs (Event/Debug).
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashSettings
{
	static const string SETTINGS_PATH = "$profile:M4D_AirPlaneCrash/PlaneCrashSettings.json";
	static const string FOLDER_PATH = "$profile:M4D_AirPlaneCrash";

	int EnableMod;
	int MaxActivePlaneEvents;
	int SafeRadius;
	int DistanceRadius;
	int CleanupRadius;
	int EnableCrashNotification;
	
	// Logs de Legado
	int EnableDebugLogging;
	int AlsoLogToRPT;
	
	// NOVAS FLAGS: Sistema PADRÃO OURO (Log Administrativo estruturado por evento)
	int EnableEventLogging;
	int EnableEventLogPerEvent;
	int EnableDebugLogPerEvent;
	int LogPlayerInteractions;
	int LogAssetCoordinates;
	int LogThreatDetails;
	int LogSoundDispatch;
	int LogWorldStateSnapshots;

	int EnableWreckPathgraphUpdate;
	int EnableContainerPathgraphUpdate;
	int EnableZombiePathgraphUpdate;
	int EnableContainerBlue;
	int EnableContainerRed;
	int EnableContainerYellow;
	int EnableContainerOrange;
	string PreferredContainer;
	int ZombieCount;
	int EnableZombieKeyDrop;
	
	// MECANICA DE CONSUMO DE CHAVES (Nova Variavel com valor fantasma para injeção automática)
	int DestroyContainerKeyOnUse = -1;
	
	int MinLootItems;
	int MaxLootItems;
	
	// Consolidação Matemática da MilitaryBox
	int MinMilitaryBoxItems;
	int MaxMilitaryBoxItems;
	
	int PristineLoot;
	int EnableOpeningAlarm;
	
	int EnableDefaultLootItems;
	int EnableDefaultMilitaryBoxLoot;

	private static ref M4D_PlaneCrashSettings s_Instance;

	static M4D_PlaneCrashSettings Get()
	{
		if (!s_Instance)
		{
			s_Instance = new M4D_PlaneCrashSettings();
			s_Instance.Load();

			// IMPORTANTE: Força a leitura dos outros dois ficheiros ao iniciar o servidor!
			M4D_PlaneCrashSites.Get();
			M4D_PlaneCrashLoot.Get();
		}
		return s_Instance;
	}

	void Load()
	{
		if (!FileExist(FOLDER_PATH)) 
		{ 
			MakeDirectory(FOLDER_PATH); 
		}

		if (FileExist(SETTINGS_PATH)) 
		{
			string errorMessage;
			if (!JsonFileLoader<M4D_PlaneCrashSettings>.LoadFile(SETTINGS_PATH, this, errorMessage)) 
			{
				Print("[M4D_PlaneCrash] ERROR: Falha ao ler PlaneCrashSettings.json! Restaurando backup nativo...");
				BuildDefaultSettings();
				Save();
			}
		} 
		else 
		{
			BuildDefaultSettings();
			Save();
		}
		
		ValidateSettings();
	}

	void Save() 
	{ 
		JsonFileLoader<M4D_PlaneCrashSettings>.JsonSaveFile(SETTINGS_PATH, this); 
	}

	void ValidateSettings()
	{
		bool needsResave = false;

		if (DestroyContainerKeyOnUse == -1) 
		{
			DestroyContainerKeyOnUse = 1; 
			needsResave = true;
			M4D_PlaneCrashLogger.Warn("[Settings] JSON antigo detectado. Injetando 'DestroyContainerKeyOnUse' automaticamente.");
		}

		if (MaxActivePlaneEvents > 10) 
		{ 
			MaxActivePlaneEvents = 3; 
			M4D_PlaneCrashLogger.Warn("[Settings] MaxActivePlaneEvents excedeu o limite de 10. Restaurado para o padrao: 3.");
		}
		
		if (SafeRadius <= 200) { SafeRadius = 1000; }
		if (DistanceRadius <= 200) { DistanceRadius = 1000; }
		if (CleanupRadius <= 200) { CleanupRadius = 1000; }

		if (needsResave == true)
		{
			Save();
		}
	}

	void BuildDefaultSettings()
	{
		EnableMod = 1;
		MaxActivePlaneEvents = 3;
		SafeRadius = 1000;
		DistanceRadius = 1000;
		CleanupRadius = 1000;
		EnableCrashNotification = 1;
		
		EnableDebugLogging = 0;
		AlsoLogToRPT = 1;
		
		// PADRÃO OURO: Inicializacao das Flags do Novo Sistema
		EnableEventLogging = 1;
		EnableEventLogPerEvent = 1;
		EnableDebugLogPerEvent = 1;
		LogPlayerInteractions = 1;
		LogAssetCoordinates = 1;
		LogThreatDetails = 1;
		LogSoundDispatch = 0;
		LogWorldStateSnapshots = 1;

		EnableWreckPathgraphUpdate = 1;
		EnableContainerPathgraphUpdate = 1;
		EnableZombiePathgraphUpdate = 1;
		EnableContainerBlue = 1;
		EnableContainerRed = 1;
		EnableContainerYellow = 1;
		EnableContainerOrange = 1;
		PreferredContainer = "Random";
		ZombieCount = 15;
		EnableZombieKeyDrop = 1;
		
		// Padrão 1 = Consumo da chave correta ativado (Gameplay Hardcore)
		DestroyContainerKeyOnUse = 1;
		
		MinLootItems = 2;
		MaxLootItems = 5;
		
		// Injeção Matemática e Estrutural da Military Box Unificada
		MinMilitaryBoxItems = 10;
		MaxMilitaryBoxItems = 17;
		
		PristineLoot = 1;
		EnableOpeningAlarm = 1;
		
		EnableDefaultLootItems = 0;
		EnableDefaultMilitaryBoxLoot = 0;
	}
}
