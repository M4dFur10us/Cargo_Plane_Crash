// =====================================================================================
// config.cpp - VERSÃO FINAL CONSOLIDADA (Caminhos Corrigidos e Sem Herança)
// Responsabilidade: Registro de metadados, dependências, objetos e mapeamento de som.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE DE REGISTRO DE MOD (CfgPatches)
	// ---------------------------------------------------------------------------------
	// Registra a existência do mod perante o servidor e define as dependências base 
	// necessárias para que a engine carregue e interprete os recursos nativos do DayZ.
	// ---------------------------------------------------------------------------------
class CfgPatches
{
    class M4D_PlaneEvent
    {
        units[] = {};
        weapons[] = {};
        requiredAddons[] = 
		{
			"DZ_Data",
			"DZ_Sounds_Effects",
			"DZ_Gear_Camping"
		};
    };
};

	// ---------------------------------------------------------------------------------
	// CLASSE DE MÓDULOS DE SCRIPT (CfgMods)
	// ---------------------------------------------------------------------------------
	// Mapeia fisicamente os diretórios do seu mod para dentro das camadas de execução 
	// da engine (GameLib, World, Mission, etc), permitindo que o Enforce Script leia os arquivos .c.
	// ---------------------------------------------------------------------------------
class CfgMods
{
    class M4D_PlaneEvent
    {
        dir = "M4d_AirPlaneCrash";
        name = "M4D AirPlane Crash Event";
		author="xXM4dFur10usXx";
        picture = "";
        action = "";
        hideName = 1;
        hidePicture = 1;
        version = "1.0";
        type = "mod";
        dependencies[] = 
		{
			"Game",
			"World",
			"Mission"
		};
        class defs
        {
			class engineScriptModule
			{
				files[]=
				{
					"M4d_AirPlaneCrash/PlaneEvent/scripts/Common"
				};
			};
			class gameLibScriptModule
			{
				files[]=
				{
					"M4d_AirPlaneCrash/PlaneEvent/scripts/Common"
				};
			};
            class gameScriptModule
            {
                files[] = 
				{
					"M4d_AirPlaneCrash/PlaneEvent/scripts/Common"
				};
            };
            class worldScriptModule
            {
                files[] = 
				{
					"M4d_AirPlaneCrash/PlaneEvent/scripts/Common",
					"M4d_AirPlaneCrash/PlaneEvent/scripts/4_World"
				};
            };
			class missionScriptModule
            {
                files[] = 
				{
					"M4d_AirPlaneCrash/PlaneEvent/scripts/Common",
					"M4d_AirPlaneCrash/PlaneEvent/scripts/5_Mission"
				};
            };
        };
    };
};

	// ---------------------------------------------------------------------------------
	// CLASSE DE VEÍCULOS E OBJETOS (CfgVehicles)
	// ---------------------------------------------------------------------------------
	// Onde a engine reconhece as instâncias físicas. Aqui o avião, os contentores e os 
	// baús de recompensa ganham volume 3D, peso, slots e capacidade de cargo.
	// ---------------------------------------------------------------------------------
class CfgVehicles
{
	class Land_Wreck_C130J_Cargo;
	class Land_ContainerLocked_Red_DE;
	class Land_ContainerLocked_Blue_DE;
	class Land_ContainerLocked_Yellow_DE;
	class Land_ContainerLocked_Orange_DE;
	class SeaChest;

	// ---------------------------------------------------------------------------------
	// CLASSE: M4d_AirPlaneCrash (A Fuselagem)
	// ---------------------------------------------------------------------------------
	class M4d_AirPlaneCrash: Land_Wreck_C130J_Cargo
	{
		scope=2;
		forceNavMesh=1;
		storageCategory=4;
	};

	// ---------------------------------------------------------------------------------
	// CLASSES DE CONTENTORES (M4D_WreckContainer...)
	// ---------------------------------------------------------------------------------
	class M4D_WreckContainerRed: Land_ContainerLocked_Red_DE
	{
		scope=2;
		forceNavMesh=1;
	};

	class M4D_WreckContainerBlue: Land_ContainerLocked_Blue_DE
	{
		scope=2;
		forceNavMesh=1;
	};

	class M4D_WreckContainerYellow: Land_ContainerLocked_Yellow_DE
	{
		scope=2;
		forceNavMesh=1;
	};

	class M4D_WreckContainerOrange: Land_ContainerLocked_Orange_DE
	{
		scope=2;
		forceNavMesh=1;
	};

	// ---------------------------------------------------------------------------------
	// CLASSE: M4D_CrashRewardChest (A Recompensa Dinâmica)
	// ---------------------------------------------------------------------------------
	class M4D_CrashRewardChest: SeaChest
	{
		scope = 2;
		displayName = "Caixa de Suprimentos Abatida";
		descriptionShort = "Conteúdo recuperado do acidente aéreo. Apenas retirada permitida.";
		model = "\DZ\gear\camping\AnniversaryBox.p3d";
		
		weight = 500000;
		itemSize[] = {10,10};
		itemBehaviour = 0;
		canBeHeldInHands = 0;
		canBePlacedInCargo = 0;
		heavyItem = 1;
		
		inventorySlot[] = {}; 

		class Cargo
		{
			itemsCargoSize[] = {10,150};
			openable = 0;
			allowOwnedCargoManipulation = 1;
		};
	};

	// ---------------------------------------------------------------------------------
	// CLASSES DE ARMAZENAMENTO FIXO (Munição e Ferramentas)
	// ---------------------------------------------------------------------------------
	class M4DCrashStorage_MilitaryBox: SeaChest
    {
        scope = 2;
        model = "DZ\structures\Military\Misc\Misc_SupplyBox2.p3d";
        displayName = "Caixa de Suprimentos Militar";
        descriptionShort = "Caixa militar de suprimentos.";
        canBeHeldInHands = 0;
        canBePlacedInCargo = 0;
        inventorySlot[] = {}; 
        class Cargo
        {
            itemsCargoSize[] = {10,100};
            openable = 0;
            allowOwnedCargoManipulation = 1;
        };
        weight = 500000;
        itemBehaviour = 0;
        itemSize[] = {10,10};
        physLayer = "item_large";
		// Parâmetro inserido para subverter a guilhotina de renderização da engine.
        // Obriga o servidor a enviar a malha visual desta caixa para a bolha Far (1000m+)
        // em vez da bolha Near (150m) padrão de objetos do tipo SeaChest/ItemBase.
		forceFarBubble = "true";
    };
};

	// ---------------------------------------------------------------------------------
	// CLASSE DE SHADERS DE SOM (CfgSoundshaders)
	// ---------------------------------------------------------------------------------
	// Vincula o nome interno da engine ao arquivo físico. A herança foi removida para 
	// evitar que updates do DayZ quebrem o som. Barra inicial removida.
	// ---------------------------------------------------------------------------------
class CfgSoundshaders
{
	// Chamada da classe base nativa da Bohemia para o Helicóptero
	class HeliCrash_Distant_SoundShader;
	
	// Nossa classe herda a física base do Helicóptero
	class PlaneCrash_SoundShader: HeliCrash_Distant_SoundShader
	{
		samples[]=
		{
			{
				// CORREÇÃO: Inserção da barra (\) no início do caminho absoluto
				"\M4d_AirPlaneCrash\PlaneEvent\Sound\PlaneCrash",
				1
			}
		};
		volume=1.5;
		range=3500;
	};
	
	class PlaneCrash_UnlockAlarm_SoundShader: HeliCrash_Distant_SoundShader
	{
		samples[]=
		{
			{
				// CORREÇÃO: Inserção da barra (\) no início do caminho absoluto
				"\M4d_AirPlaneCrash\PlaneEvent\Sound\AlarmDoorOpenning",
				1
			}
		};
		volume=1.0;
		range=1500; // Alarme toca em uma distância menor que a queda
	};
};

	// ---------------------------------------------------------------------------------
	// CLASSE DE CONJUNTOS DE SOM (CfgSoundsets)
	// ---------------------------------------------------------------------------------
	// Agrupa o Shader e define as propriedades espaciais independentes e blindadas.
	// ---------------------------------------------------------------------------------
class CfgSoundsets
{
	// Chamada da classe base nativa da Bohemia para o Helicóptero
	class HeliCrash_Distant_Base_SoundSet;
	
	// Nossa classe herda o conjunto de filtros e curvas 3D da Bohemia
	class PlaneCrash_SoundSet: HeliCrash_Distant_Base_SoundSet
	{
		soundShaders[]=
		{
			"PlaneCrash_SoundShader"
		};
		// Tipo de processamento usado para sons de trovões/explosões à distância
		sound3DProcessingType="ThunderNear3DProcessingType";
	};
	
	class PlaneCrash_UnlockAlarm_SoundSet: HeliCrash_Distant_Base_SoundSet
	{
		soundShaders[]=
		{
			"PlaneCrash_UnlockAlarm_SoundShader"
		};
		// Tipo de processamento usado para sons de trovões/explosões à distância
		sound3DProcessingType="ThunderNear3DProcessingType";
	};
};