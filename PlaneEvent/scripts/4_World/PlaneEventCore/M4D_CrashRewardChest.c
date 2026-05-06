// =====================================================================================
// M4D_CrashRewardChest.c - VERSÃO FINAL CONSOLIDADA (Visibilidade e Acesso Livre)
// Responsabilidade: Entidade física do prêmio com controle de visibilidade de inventário.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// BAÚ DE RECOMPENSA PRINCIPAL (M4D_CrashRewardChest)
	// ---------------------------------------------------------------------------------
	// Entidade física que armazena o loot de alto valor (AnniversaryBox) no interior do contentor.
	// É responsável por gerenciar a visibilidade sincronizada do inventário na rede para o jogador
	// e bloquear a inserção de itens de fora, permitindo sempre a retirada livre do conteúdo.
	// ---------------------------------------------------------------------------------
class M4D_CrashRewardChest extends SeaChest 
{
	protected int m_M4DOwnerEventID = -1;
	protected bool m_M4D_IsSystemSpawningLoot = false;
	protected bool m_M4D_IsInventoryVisible = false; 

	// ---------------------------------------------------------------------------------
	// CONSTRUTOR: REGISTRO DE SINCRONIZAÇÃO DE REDE
	// ---------------------------------------------------------------------------------
	void M4D_CrashRewardChest()
	{
		RegisterNetSyncVariableBool("m_M4D_IsInventoryVisible");
	}

	void SetOwnerEventID(int id) 
	{ 
		m_M4DOwnerEventID = id; 
	}

	int GetOwnerEventID() 
	{ 
		return m_M4DOwnerEventID; 
	}

	void SetSystemSpawningMode(bool state) 
	{ 
		m_M4D_IsSystemSpawningLoot = state; 
	}

	// ---------------------------------------------------------------------------------
	// CONTROLE DE VISIBILIDADE (Acionado pelo Contêiner Principal)
	// ---------------------------------------------------------------------------------
	void SetInventoryVisibility(bool state) 
	{
		m_M4D_IsInventoryVisible = state;
		SetSynchDirty(); 
	}
	
	override bool IsInventoryVisible() 
	{ 
		return m_M4D_IsInventoryVisible;
	}

	// ---------------------------------------------------------------------------------
	// RESTRIÇÃO DE CARGO (Apenas Retirada Permitida para Jogadores)
	// ---------------------------------------------------------------------------------
	override bool CanReceiveItemIntoCargo(EntityAI item) 
	{ 
		if (m_M4D_IsSystemSpawningLoot)
		{
			return true;
		}
		
		return false;
	}
	
	override bool CanPutInCargo(EntityAI parent) 
	{ 
		return false; 
	}
	
	override bool CanPutIntoHands(EntityAI parent) 
	{ 
		return false; 
	}

	// ---------------------------------------------------------------------------------
	// PERSISTÊNCIA E REIDRATAÇÃO (Serialização do Snapshot)
	// ---------------------------------------------------------------------------------
	override void OnStoreSave(ParamsWriteContext ctx) 
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_M4DOwnerEventID);
		ctx.Write(m_M4D_IsInventoryVisible);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version) 
	{ 
		if (!super.OnStoreLoad(ctx, version)) 
		{
			return false;
		}
		if (!ctx.Read(m_M4DOwnerEventID)) 
		{
			m_M4DOwnerEventID = -1;
		}
		if (!ctx.Read(m_M4D_IsInventoryVisible)) 
		{
			m_M4D_IsInventoryVisible = false;
		}
		return true;
	}

	// ---------------------------------------------------------------------------------
	// LOG DE SEGURANÇA E AUDITORIA (Monitoramento de Retirada)
	// ---------------------------------------------------------------------------------
	override void EEItemDetached(EntityAI item, string slot_name)
	{
		super.EEItemDetached(item, slot_name);
		
		if (GetGame() && GetGame().IsServer())
		{
			M4D_PlaneCrashLogger.Info(string.Format("[EventID: %1] Item retirado do bau de recompensa: %2", m_M4DOwnerEventID, item.GetType()));
		}
	}
}