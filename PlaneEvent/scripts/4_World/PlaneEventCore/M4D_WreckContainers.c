// =====================================================================================
// M4D_WreckContainers.c - VERSÃO DEFINITIVA (Mecânica de Consumo de Chaves)
// Responsabilidade: Contentores do Evento, Caixas de Loot Secundárias e RPC de Áudio.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// FUNÇÃO GLOBAL DE AUDITORIA (M4D_GetNearestPlayerLog)
	// ---------------------------------------------------------------------------------
	// Usada para identificar qual jogador destrancou o contentor, registrando o nome e 
	// o SteamID nos logs administrativos do servidor em formato vertical.
	// ---------------------------------------------------------------------------------
string M4D_GetNearestPlayerLog(vector pos, float maxRadius = 10.0)
{
	string playerName = "Desconhecido";
	string steamId = "N/A";
	array<Man> players = new array<Man>();
	GetGame().GetPlayers(players);
	
	for (int pIdx = 0; pIdx < players.Count(); pIdx++) 
	{
		Man p = players.Get(pIdx);
		if (p && p.IsAlive() && vector.Distance(pos, p.GetPosition()) <= maxRadius) 
		{
			PlayerBase pb = PlayerBase.Cast(p);
			if (pb && pb.GetIdentity()) 
			{
				playerName = pb.GetIdentity().GetName();
				steamId = pb.GetIdentity().GetId();
				break;
			}
		}
	}
	return string.Format("Player: %1\nSteamID: %2", playerName, steamId);
}

	// ---------------------------------------------------------------------------------
	// MAPEAMENTO DE CHAVES (M4D_GetExpectedKeyTypeForContainer)
	// ---------------------------------------------------------------------------------
	// Retorna a classe exata da chave vanilla exigida para abrir o respectivo contentor.
	// ---------------------------------------------------------------------------------
string M4D_GetExpectedKeyTypeForContainer(string containerType)
{
	if (containerType.Contains("Blue")) 
	{ 
		return "ShippingContainerKeys_Blue"; 
	}
	if (containerType.Contains("Red")) 
	{ 
		return "ShippingContainerKeys_Red"; 
	}
	if (containerType.Contains("Yellow")) 
	{ 
		return "ShippingContainerKeys_Yellow"; 
	}
	if (containerType.Contains("Orange")) 
	{ 
		return "ShippingContainerKeys_Orange"; 
	}
	return "";
}

	// ---------------------------------------------------------------------------------
	// BUSCA DE ENTIDADE DO JOGADOR (M4D_GetNearestPlayerBase)
	// ---------------------------------------------------------------------------------
	// Localiza e retorna a entidade física (PlayerBase) do jogador mais próximo.
	// Necessário para interagir com o inventário e remover a chave das mãos.
	// ---------------------------------------------------------------------------------
PlayerBase M4D_GetNearestPlayerBase(vector pos, float maxRadius = 10.0)
{
	array<Man> players = new array<Man>();
	GetGame().GetPlayers(players);
	
	for (int i = 0; i < players.Count(); i++) 
	{
		Man p = players.Get(i);
		if (p && p.IsAlive() && vector.Distance(pos, p.GetPosition()) <= maxRadius) 
		{
			return PlayerBase.Cast(p);
		}
	}
	return null;
}

	// ---------------------------------------------------------------------------------
	// MOTOR DE CONSUMO DE CHAVES (M4D_ConsumeCorrectContainerKeyIfEnabled)
	// ---------------------------------------------------------------------------------
	// Valida a configuração do servidor e, se habilitado, apaga a chave correta
	// das mãos do jogador após o desbloqueio bem sucedido, registrando o log.
	// ---------------------------------------------------------------------------------
void M4D_ConsumeCorrectContainerKeyIfEnabled(vector pos, int eventId, string containerType)
{
	if (!GetGame().IsServer()) 
	{
		return;
	}

	ref M4D_PlaneCrashSettings settings = M4D_PlaneCrashSettings.Get();
	if (!settings) 
	{
		return;
	}
	
	if (settings.DestroyContainerKeyOnUse != 1) 
	{
		return;
	}

	string expectedKey = M4D_GetExpectedKeyTypeForContainer(containerType);
	PlayerBase player = M4D_GetNearestPlayerBase(pos);

	if (player)
	{
		if (expectedKey != "")
		{
			EntityAI itemInHand = player.GetHumanInventory().GetEntityInHands();
			if (itemInHand)
			{
				if (itemInHand.GetType() == expectedKey)
				{
					string playerName = "Desconhecido";
					string steamId = "N/A";
					
					if (player.GetIdentity())
					{
						playerName = player.GetIdentity().GetName();
						steamId = player.GetIdentity().GetId();
					}
					
					string keyLog = string.Format("\n[KEY CONSUMED]\nEventID: %1\nPlayer: %2\nSteamID: %3\nContainerType: %4\nKeyType: %5\nReason: correct key consumed after successful unlock", eventId, playerName, steamId, containerType, expectedKey);
					
					M4D_PlaneCrashLogger.EventContainer(eventId, keyLog);
					GetGame().ObjectDelete(itemInHand); 
				}
			}
		}
	}
}

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DO CONTENTOR VERMELHO (M4D_WreckContainerRed)
	// ---------------------------------------------------------------------------------
	// Casca protetora do evento. Gerencia seu tempo de vida e capta o uso da chave
	// para acionar o baú de recompensa interno e despachar o alarme via RPC.
	// ---------------------------------------------------------------------------------
class M4D_WreckContainerRed extends Land_ContainerLocked_Red_DE 
{
	protected bool   m_M4DIsUnlocked = false;
	protected int    m_M4DOwnerEventID = -1;
	protected int    m_M4DElapsedMs = 0;    
	protected bool   m_M4DTimerArmed = false; 
	protected bool   m_M4DAlarmActive = false; 

	static const int   M4D_MAX_CONTAINER_LIFETIME = 1800000; 
	static const int   M4D_CHECK_INTERVAL = 100000;  

	void M4D_WreckContainerRed()
	{
		RegisterNetSyncVariableBool("m_M4DIsUnlocked");
		RegisterNetSyncVariableBool("m_M4DAlarmActive");
	}

	void SetOwnerEventID(int id) 
	{ 
		m_M4DOwnerEventID = id; 
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkContainerAlive(id, this.GetType());
		}
	}
	
	int GetOwnerEventID() 
	{ 
		return m_M4DOwnerEventID; 
	}
	
	bool GetUnlockedState() 
	{ 
		return m_M4DIsUnlocked; 
	}

	override void EEInit() 
	{
		super.EEInit();
		if (GetGame().IsServer()) 
		{
			ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
			if (s && s.EnableMod == 1) 
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DelayedInit, 100, false);
				M4D_StartTimerIfNeeded();
			}
			else if (s && s.EnableMod == 0)
			{
				GetGame().ObjectDelete(this);
				return;
			}
		}
	}

	void DelayedInit() 
	{
		if (GetInventory()) 
		{
			GetInventory().LockInventory(LOCK_FROM_SCRIPT);
		}
	}

	override void EEDelete(EntityAI parent) 
	{
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkContainerDead(m_M4DOwnerEventID);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.M4D_TimerTick);
		}
		super.EEDelete(parent);
	}

	// ---------------------------------------------------------------------------------
	// PERSISTÊNCIA DE DADOS (OnStoreSave)
	// ---------------------------------------------------------------------------------
	// Ação 1 Aplicada: Remoção do salvamento de m_M4DTimerArmed para evitar amnésia de
	// CallQueue e garantir o reinício do timer após o boot do servidor.
	// ---------------------------------------------------------------------------------
	override void OnStoreSave(ParamsWriteContext ctx) 
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_M4DElapsedMs); 
		ctx.Write(m_M4DIsUnlocked);
		ctx.Write(m_M4DOwnerEventID); 
		ctx.Write(m_M4DAlarmActive);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version) 
	{
		if (!super.OnStoreLoad(ctx, version)) 
		{
			return false;
		}
		ctx.Read(m_M4DElapsedMs); 
		ctx.Read(m_M4DIsUnlocked);
		
		if (!ctx.Read(m_M4DOwnerEventID)) 
		{
			m_M4DOwnerEventID = -1;
		}
		if (!ctx.Read(m_M4DAlarmActive)) 
		{
			m_M4DAlarmActive = false;
		}
		
		if (GetGame().IsServer()) 
		{
			M4D_StartTimerIfNeeded();
		}
		return true;
	}

	void M4D_StartTimerIfNeeded() 
	{
		if (!m_M4DTimerArmed) 
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_TimerTick, M4D_CHECK_INTERVAL, true);
			m_M4DTimerArmed = true;
		}
	}

	protected void M4D_TimerTick() 
	{
		if (!GetGame().IsServer()) 
		{
			return;
		}
		
		ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
		if (!s) 
		{
			return;
		}

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		bool nearby = false;
		
		for (int i = 0; i < players.Count(); i++) 
		{
			if (players.Get(i) && vector.Distance(GetPosition(), players.Get(i).GetPosition()) <= s.CleanupRadius) 
			{
				nearby = true; 
				break;
			}
		}
		
		if (!nearby) 
		{
			m_M4DElapsedMs += M4D_CHECK_INTERVAL;
			if (m_M4DElapsedMs >= M4D_MAX_CONTAINER_LIFETIME) 
			{
				GetGame().ObjectDelete(this);
			}
		}
	}

	// ---------------------------------------------------------------------------------
	// GATILHO DE DESTRANCAMENTO (OnDoorUnlocked)
	// ---------------------------------------------------------------------------------
	// Executa a liberação física do inventário e despacha a onda de som do alarme
	// calculando a velocidade do som em relação à distância de cada jogador no raio.
	// ---------------------------------------------------------------------------------
	override void OnDoorUnlocked(DoorLockParams params) 
	{
		super.OnDoorUnlocked(params);
		int doorIdx = params.param1;
		m_LockedMask &= ~(1 << doorIdx);
		SetAnimationPhase(string.Format("side%1_lock", doorIdx + 1), 1);

		if (!m_M4DIsUnlocked) 
		{
			m_M4DIsUnlocked = true;
			SetSynchDirty(); 
			
			if (GetGame() && GetGame().IsServer()) 
			{
				M4D_PlaneCrashWorldState.MarkContainerUnlocked(m_M4DOwnerEventID);
				
				array<Object> objs = new array<Object>();
				GetGame().GetObjectsAtPosition(GetPosition(), 10.0, objs, null);
				
				for (int i = 0; i < objs.Count(); i++) 
				{
					M4D_CrashRewardChest chest = M4D_CrashRewardChest.Cast(objs.Get(i));
					if (chest) 
					{
						if (chest.GetOwnerEventID() == m_M4DOwnerEventID) 
						{
							chest.SetInventoryVisibility(true);
							M4D_PlaneCrashLogger.TraceObject(m_M4DOwnerEventID, "Inventario do bau agora visivel para coleta.");
							break;
						}
					}
				}
			}

			if (!GetGame().IsDedicatedServer()) 
			{
				SEffectManager.PlaySound("Land_ContainerLocked_lock_SoundSet", GetPosition());
			}

			if (GetGame().IsServer()) 
			{
				string playerInfo = M4D_GetNearestPlayerLog(GetPosition());
				string unlockedLog = string.Format("\n[CONTAINER UNLOCKED]\nEventID: %1\n%2\nContainerType: %3\nRewardChest: unlocked", m_M4DOwnerEventID, playerInfo, this.GetType());
				M4D_PlaneCrashLogger.EventContainer(m_M4DOwnerEventID, unlockedLog);
				
				M4D_ConsumeCorrectContainerKeyIfEnabled(GetPosition(), m_M4DOwnerEventID, this.GetType());
				
				// BLOCO DE PROPAGAÇÃO DE SOM (Speed of Sound)
				ref M4D_PlaneCrashSettings settings = M4D_PlaneCrashSettings.Get();
				if (!settings || settings.EnableOpeningAlarm == 1) 
				{
					int playersReached = 0;
					const int M4D_RPC_ALARM_SOUND = 4857393; 
					array<Man> playersForSound = new array<Man>(); 
					GetGame().GetPlayers(playersForSound);
					vector wPos = GetPosition();
					
					for (int pIdx = 0; pIdx < playersForSound.Count(); pIdx++) 
					{
						PlayerBase playerToHear = PlayerBase.Cast(playersForSound.Get(pIdx));
						if (playerToHear)
						{
							if (playerToHear.GetIdentity()) 
							{
								float distance = vector.Distance(playerToHear.GetPosition(), wPos);
								if (distance <= 1500.0) 
								{
									playersReached = playersReached + 1;
									float speedOfSound = 343.0; 
									float delaySeconds = distance / speedOfSound;
									int delayMilliseconds = Math.Round(delaySeconds * 1000);
									GetGame().RPCSingleParam(playerToHear, M4D_RPC_ALARM_SOUND, new Param2<vector, int>(wPos, delayMilliseconds), true, playerToHear.GetIdentity());
								}
							}
						}
					}
					
					m_M4DAlarmActive = true; 
					SetSynchDirty();

					string alarmLog = string.Format("\n[ALARM DISPATCHED]\nEventID: %1\nRadius: 1500m\nPlayersReached: %2", m_M4DOwnerEventID, playersReached);
					M4D_PlaneCrashLogger.EventContainer(m_M4DOwnerEventID, alarmLog);
				}
			}
		}
	}

	override void OnVariablesSynchronized() 
	{
		super.OnVariablesSynchronized();
		
		if (GetAnimationPhase("Doors1") > 0 || GetAnimationPhase("Doors2") > 0) 
		{
			// Estrutura retida por regra de integridade. Alarme movido para OnDoorUnlocked.
		}
	}

	override bool CanReleaseAttachment(EntityAI attachment) { return false; }
	override bool CanReceiveAttachment(EntityAI attachment, int slotId) { return true; }
	override bool IsInventoryVisible() { return !GetInventory().IsInventoryLockedForLockType(LOCK_FROM_SCRIPT); }
}

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DO CONTENTOR AZUL (M4D_WreckContainerBlue)
	// ---------------------------------------------------------------------------------
class M4D_WreckContainerBlue extends Land_ContainerLocked_Blue_DE 
{
	protected bool   m_M4DIsUnlocked = false;
	protected int    m_M4DOwnerEventID = -1;
	protected int    m_M4DElapsedMs = 0;    
	protected bool   m_M4DTimerArmed = false; 
	protected bool   m_M4DAlarmActive = false;

	static const int   M4D_MAX_CONTAINER_LIFETIME = 1800000; 
	static const int   M4D_CHECK_INTERVAL = 100000;  

	void M4D_WreckContainerBlue()
	{
		RegisterNetSyncVariableBool("m_M4DIsUnlocked");
		RegisterNetSyncVariableBool("m_M4DAlarmActive");
	}

	void SetOwnerEventID(int id) 
	{ 
		m_M4DOwnerEventID = id; 
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkContainerAlive(id, this.GetType());
		}
	}
	
	int GetOwnerEventID() 
	{ 
		return m_M4DOwnerEventID; 
	}
	
	bool GetUnlockedState() 
	{ 
		return m_M4DIsUnlocked; 
	}

	override void EEInit() 
	{
		super.EEInit();
		if (GetGame().IsServer()) 
		{
			ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
			if (s && s.EnableMod == 1) 
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DelayedInit, 100, false);
				M4D_StartTimerIfNeeded();
			}
			else if (s && s.EnableMod == 0)
			{
				GetGame().ObjectDelete(this);
				return;
			}
		}
	}

	void DelayedInit() 
	{
		if (GetInventory()) 
		{
			GetInventory().LockInventory(LOCK_FROM_SCRIPT);
		}
	}

	override void EEDelete(EntityAI parent) 
	{
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkContainerDead(m_M4DOwnerEventID);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.M4D_TimerTick);
		}
		super.EEDelete(parent);
	}

	override void OnStoreSave(ParamsWriteContext ctx) 
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_M4DElapsedMs); 
		ctx.Write(m_M4DIsUnlocked);
		ctx.Write(m_M4DOwnerEventID); 
		ctx.Write(m_M4DAlarmActive);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version) 
	{
		if (!super.OnStoreLoad(ctx, version)) 
		{
			return false;
		}
		ctx.Read(m_M4DElapsedMs); 
		ctx.Read(m_M4DIsUnlocked);
		
		if (!ctx.Read(m_M4DOwnerEventID)) 
		{
			m_M4DOwnerEventID = -1;
		}
		if (!ctx.Read(m_M4DAlarmActive)) 
		{
			m_M4DAlarmActive = false;
		}
		
		if (GetGame().IsServer()) 
		{
			M4D_StartTimerIfNeeded();
		}
		return true;
	}

	void M4D_StartTimerIfNeeded() 
	{
		if (!m_M4DTimerArmed) 
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_TimerTick, M4D_CHECK_INTERVAL, true);
			m_M4DTimerArmed = true;
		}
	}

	protected void M4D_TimerTick() 
	{
		if (!GetGame().IsServer()) 
		{
			return;
		}
		
		ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
		if (!s) 
		{
			return;
		}

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		bool nearby = false;
		
		for (int i = 0; i < players.Count(); i++) 
		{
			if (players.Get(i) && vector.Distance(GetPosition(), players.Get(i).GetPosition()) <= s.CleanupRadius) 
			{
				nearby = true; 
				break;
			}
		}
		
		if (!nearby) 
		{
			m_M4DElapsedMs += M4D_CHECK_INTERVAL;
			if (m_M4DElapsedMs >= M4D_MAX_CONTAINER_LIFETIME) 
			{
				GetGame().ObjectDelete(this);
			}
		}
	}

	override void OnDoorUnlocked(DoorLockParams params) 
	{
		super.OnDoorUnlocked(params);
		int doorIdx = params.param1;
		m_LockedMask &= ~(1 << doorIdx);
		SetAnimationPhase(string.Format("side%1_lock", doorIdx + 1), 1);

		if (!m_M4DIsUnlocked) 
		{
			m_M4DIsUnlocked = true;
			SetSynchDirty(); 
			
			if (GetGame() && GetGame().IsServer()) 
			{
				M4D_PlaneCrashWorldState.MarkContainerUnlocked(m_M4DOwnerEventID);
				
				array<Object> objs = new array<Object>();
				GetGame().GetObjectsAtPosition(GetPosition(), 10.0, objs, null);
				
				for (int i = 0; i < objs.Count(); i++) 
				{
					M4D_CrashRewardChest chest = M4D_CrashRewardChest.Cast(objs.Get(i));
					if (chest) 
					{
						if (chest.GetOwnerEventID() == m_M4DOwnerEventID) 
						{
							chest.SetInventoryVisibility(true);
							M4D_PlaneCrashLogger.TraceObject(m_M4DOwnerEventID, "Inventario do bau agora visivel para coleta.");
							break;
						}
					}
				}
			}

			if (!GetGame().IsDedicatedServer()) 
			{
				SEffectManager.PlaySound("Land_ContainerLocked_lock_SoundSet", GetPosition());
			}

			if (GetGame().IsServer()) 
			{
				string playerInfo = M4D_GetNearestPlayerLog(GetPosition());
				string unlockedLog = string.Format("\n[CONTAINER UNLOCKED]\nEventID: %1\n%2\nContainerType: %3\nRewardChest: unlocked", m_M4DOwnerEventID, playerInfo, this.GetType());
				M4D_PlaneCrashLogger.EventContainer(m_M4DOwnerEventID, unlockedLog);
				
				M4D_ConsumeCorrectContainerKeyIfEnabled(GetPosition(), m_M4DOwnerEventID, this.GetType());
				
				ref M4D_PlaneCrashSettings settings = M4D_PlaneCrashSettings.Get();
				if (!settings || settings.EnableOpeningAlarm == 1) 
				{
					int playersReached = 0;
					const int M4D_RPC_ALARM_SOUND = 4857393; 
					array<Man> playersForSound = new array<Man>(); 
					GetGame().GetPlayers(playersForSound);
					vector wPos = GetPosition();
					
					for (int pIdx = 0; pIdx < playersForSound.Count(); pIdx++) 
					{
						PlayerBase playerToHear = PlayerBase.Cast(playersForSound.Get(pIdx));
						if (playerToHear)
						{
							if (playerToHear.GetIdentity()) 
							{
								float distance = vector.Distance(playerToHear.GetPosition(), wPos);
								if (distance <= 1500.0) 
								{
									playersReached = playersReached + 1;
									float speedOfSound = 343.0; 
									float delaySeconds = distance / speedOfSound;
									int delayMilliseconds = Math.Round(delaySeconds * 1000);
									GetGame().RPCSingleParam(playerToHear, M4D_RPC_ALARM_SOUND, new Param2<vector, int>(wPos, delayMilliseconds), true, playerToHear.GetIdentity());
								}
							}
						}
					}
					
					m_M4DAlarmActive = true; 
					SetSynchDirty();

					string alarmLog = string.Format("\n[ALARM DISPATCHED]\nEventID: %1\nRadius: 1500m\nPlayersReached: %2", m_M4DOwnerEventID, playersReached);
					M4D_PlaneCrashLogger.EventContainer(m_M4DOwnerEventID, alarmLog);
				}
			}
		}
	}

	override void OnVariablesSynchronized() 
	{
		super.OnVariablesSynchronized();
		
		if (GetAnimationPhase("Doors1") > 0 || GetAnimationPhase("Doors2") > 0) 
		{
			// Estrutura retida por regra de integridade. Alarme movido para OnDoorUnlocked.
		}
	}

	override bool CanReleaseAttachment(EntityAI attachment) { return false; }
	override bool CanReceiveAttachment(EntityAI attachment, int slotId) { return true; }
	override bool IsInventoryVisible() { return !GetInventory().IsInventoryLockedForLockType(LOCK_FROM_SCRIPT); }
}

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DO CONTENTOR AMARELO (M4D_WreckContainerYellow)
	// ---------------------------------------------------------------------------------
class M4D_WreckContainerYellow extends Land_ContainerLocked_Yellow_DE 
{
	protected bool   m_M4DIsUnlocked = false;
	protected int    m_M4DOwnerEventID = -1;
	protected int    m_M4DElapsedMs = 0;    
	protected bool   m_M4DTimerArmed = false; 
	protected bool   m_M4DAlarmActive = false;

	static const int   M4D_MAX_CONTAINER_LIFETIME = 1800000; 
	static const int   M4D_CHECK_INTERVAL = 100000;  

	void M4D_WreckContainerYellow()
	{
		RegisterNetSyncVariableBool("m_M4DIsUnlocked");
		RegisterNetSyncVariableBool("m_M4DAlarmActive");
	}

	void SetOwnerEventID(int id) 
	{ 
		m_M4DOwnerEventID = id; 
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkContainerAlive(id, this.GetType());
		}
	}
	
	int GetOwnerEventID() 
	{ 
		return m_M4DOwnerEventID; 
	}
	
	bool GetUnlockedState() 
	{ 
		return m_M4DIsUnlocked; 
	}

	override void EEInit() 
	{
		super.EEInit();
		if (GetGame().IsServer()) 
		{
			ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
			if (s && s.EnableMod == 1) 
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DelayedInit, 100, false);
				M4D_StartTimerIfNeeded();
			}
			else if (s && s.EnableMod == 0)
			{
				GetGame().ObjectDelete(this);
				return;
			}
		}
	}

	void DelayedInit() 
	{
		if (GetInventory()) 
		{
			GetInventory().LockInventory(LOCK_FROM_SCRIPT);
		}
	}

	override void EEDelete(EntityAI parent) 
	{
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkContainerDead(m_M4DOwnerEventID);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.M4D_TimerTick);
		}
		super.EEDelete(parent);
	}

	override void OnStoreSave(ParamsWriteContext ctx) 
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_M4DElapsedMs); 
		ctx.Write(m_M4DIsUnlocked);
		ctx.Write(m_M4DOwnerEventID); 
		ctx.Write(m_M4DAlarmActive);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version) 
	{
		if (!super.OnStoreLoad(ctx, version)) 
		{
			return false;
		}
		ctx.Read(m_M4DElapsedMs); 
		ctx.Read(m_M4DIsUnlocked);
		
		if (!ctx.Read(m_M4DOwnerEventID)) 
		{
			m_M4DOwnerEventID = -1;
		}
		if (!ctx.Read(m_M4DAlarmActive)) 
		{
			m_M4DAlarmActive = false;
		}
		
		if (GetGame().IsServer()) 
		{
			M4D_StartTimerIfNeeded();
		}
		return true;
	}

	void M4D_StartTimerIfNeeded() 
	{
		if (!m_M4DTimerArmed) 
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_TimerTick, M4D_CHECK_INTERVAL, true);
			m_M4DTimerArmed = true;
		}
	}

	protected void M4D_TimerTick() 
	{
		if (!GetGame().IsServer()) 
		{
			return;
		}
		
		ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
		if (!s) 
		{
			return;
		}

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		bool nearby = false;
		
		for (int i = 0; i < players.Count(); i++) 
		{
			if (players.Get(i) && vector.Distance(GetPosition(), players.Get(i).GetPosition()) <= s.CleanupRadius) 
			{
				nearby = true; 
				break;
			}
		}
		
		if (!nearby) 
		{
			m_M4DElapsedMs += M4D_CHECK_INTERVAL;
			if (m_M4DElapsedMs >= M4D_MAX_CONTAINER_LIFETIME) 
			{
				GetGame().ObjectDelete(this);
			}
		}
	}

	override void OnDoorUnlocked(DoorLockParams params) 
	{
		super.OnDoorUnlocked(params);
		int doorIdx = params.param1;
		m_LockedMask &= ~(1 << doorIdx);
		SetAnimationPhase(string.Format("side%1_lock", doorIdx + 1), 1);

		if (!m_M4DIsUnlocked) 
		{
			m_M4DIsUnlocked = true;
			SetSynchDirty(); 
			
			if (GetGame() && GetGame().IsServer()) 
			{
				M4D_PlaneCrashWorldState.MarkContainerUnlocked(m_M4DOwnerEventID);
				
				array<Object> objs = new array<Object>();
				GetGame().GetObjectsAtPosition(GetPosition(), 10.0, objs, null);
				
				for (int i = 0; i < objs.Count(); i++) 
				{
					M4D_CrashRewardChest chest = M4D_CrashRewardChest.Cast(objs.Get(i));
					if (chest) 
					{
						if (chest.GetOwnerEventID() == m_M4DOwnerEventID) 
						{
							chest.SetInventoryVisibility(true);
							M4D_PlaneCrashLogger.TraceObject(m_M4DOwnerEventID, "Inventario do bau agora visivel para coleta.");
							break;
						}
					}
				}
			}

			if (!GetGame().IsDedicatedServer()) 
			{
				SEffectManager.PlaySound("Land_ContainerLocked_lock_SoundSet", GetPosition());
			}

			if (GetGame().IsServer()) 
			{
				string playerInfo = M4D_GetNearestPlayerLog(GetPosition());
				string unlockedLog = string.Format("\n[CONTAINER UNLOCKED]\nEventID: %1\n%2\nContainerType: %3\nRewardChest: unlocked", m_M4DOwnerEventID, playerInfo, this.GetType());
				M4D_PlaneCrashLogger.EventContainer(m_M4DOwnerEventID, unlockedLog);
				
				M4D_ConsumeCorrectContainerKeyIfEnabled(GetPosition(), m_M4DOwnerEventID, this.GetType());
				
				ref M4D_PlaneCrashSettings settings = M4D_PlaneCrashSettings.Get();
				if (!settings || settings.EnableOpeningAlarm == 1) 
				{
					int playersReached = 0;
					const int M4D_RPC_ALARM_SOUND = 4857393; 
					array<Man> playersForSound = new array<Man>(); 
					GetGame().GetPlayers(playersForSound);
					vector wPos = GetPosition();
					
					for (int pIdx = 0; pIdx < playersForSound.Count(); pIdx++) 
					{
						PlayerBase playerToHear = PlayerBase.Cast(playersForSound.Get(pIdx));
						if (playerToHear)
						{
							if (playerToHear.GetIdentity()) 
							{
								float distance = vector.Distance(playerToHear.GetPosition(), wPos);
								if (distance <= 1500.0) 
								{
									playersReached = playersReached + 1;
									float speedOfSound = 343.0; 
									float delaySeconds = distance / speedOfSound;
									int delayMilliseconds = Math.Round(delaySeconds * 1000);
									GetGame().RPCSingleParam(playerToHear, M4D_RPC_ALARM_SOUND, new Param2<vector, int>(wPos, delayMilliseconds), true, playerToHear.GetIdentity());
								}
							}
						}
					}
					
					m_M4DAlarmActive = true; 
					SetSynchDirty();

					string alarmLog = string.Format("\n[ALARM DISPATCHED]\nEventID: %1\nRadius: 1500m\nPlayersReached: %2", m_M4DOwnerEventID, playersReached);
					M4D_PlaneCrashLogger.EventContainer(m_M4DOwnerEventID, alarmLog);
				}
			}
		}
	}

	override void OnVariablesSynchronized() 
	{
		super.OnVariablesSynchronized();
		
		if (GetAnimationPhase("Doors1") > 0 || GetAnimationPhase("Doors2") > 0) 
		{
			// Estrutura retida por regra de integridade. Alarme movido para OnDoorUnlocked.
		}
	}

	override bool CanReleaseAttachment(EntityAI attachment) { return false; }
	override bool CanReceiveAttachment(EntityAI attachment, int slotId) { return true; }
	override bool IsInventoryVisible() { return !GetInventory().IsInventoryLockedForLockType(LOCK_FROM_SCRIPT); }
}

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DO CONTENTOR LARANJA (M4D_WreckContainerOrange)
	// ---------------------------------------------------------------------------------
class M4D_WreckContainerOrange extends Land_ContainerLocked_Orange_DE 
{
	protected bool   m_M4DIsUnlocked = false;
	protected int    m_M4DOwnerEventID = -1;
	protected int    m_M4DElapsedMs = 0;    
	protected bool   m_M4DTimerArmed = false; 
	protected bool   m_M4DAlarmActive = false;

	static const int   M4D_MAX_CONTAINER_LIFETIME = 1800000; 
	static const int   M4D_CHECK_INTERVAL = 100000;  

	void M4D_WreckContainerOrange()
	{
		RegisterNetSyncVariableBool("m_M4DIsUnlocked");
		RegisterNetSyncVariableBool("m_M4DAlarmActive");
	}

	void SetOwnerEventID(int id) 
	{ 
		m_M4DOwnerEventID = id; 
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkContainerAlive(id, this.GetType());
		}
	}
	
	int GetOwnerEventID() 
	{ 
		return m_M4DOwnerEventID; 
	}
	
	bool GetUnlockedState() 
	{ 
		return m_M4DIsUnlocked; 
	}

	override void EEInit() 
	{
		super.EEInit();
		if (GetGame().IsServer()) 
		{
			ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
			if (s && s.EnableMod == 1) 
			{
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DelayedInit, 100, false);
				M4D_StartTimerIfNeeded();
			}
			else if (s && s.EnableMod == 0)
			{
				GetGame().ObjectDelete(this);
				return;
			}
		}
	}

	void DelayedInit() 
	{
		if (GetInventory()) 
		{
			GetInventory().LockInventory(LOCK_FROM_SCRIPT);
		}
	}

	override void EEDelete(EntityAI parent) 
	{
		if (GetGame() && GetGame().IsServer()) 
		{
			M4D_PlaneCrashWorldState.MarkContainerDead(m_M4DOwnerEventID);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.M4D_TimerTick);
		}
		super.EEDelete(parent);
	}

	override void OnStoreSave(ParamsWriteContext ctx) 
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_M4DElapsedMs); 
		ctx.Write(m_M4DIsUnlocked);
		ctx.Write(m_M4DOwnerEventID); 
		ctx.Write(m_M4DAlarmActive);
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version) 
	{
		if (!super.OnStoreLoad(ctx, version)) 
		{
			return false;
		}
		ctx.Read(m_M4DElapsedMs); 
		ctx.Read(m_M4DIsUnlocked);
		
		if (!ctx.Read(m_M4DOwnerEventID)) 
		{
			m_M4DOwnerEventID = -1;
		}
		if (!ctx.Read(m_M4DAlarmActive)) 
		{
			m_M4DAlarmActive = false;
		}
		
		if (GetGame().IsServer()) 
		{
			M4D_StartTimerIfNeeded();
		}
		return true;
	}

	void M4D_StartTimerIfNeeded() 
	{
		if (!m_M4DTimerArmed) 
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_TimerTick, M4D_CHECK_INTERVAL, true);
			m_M4DTimerArmed = true;
		}
	}

	protected void M4D_TimerTick() 
	{
		if (!GetGame().IsServer()) 
		{
			return;
		}
		
		ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
		if (!s) 
		{
			return;
		}

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
		bool nearby = false;
		
		for (int i = 0; i < players.Count(); i++) 
		{
			if (players.Get(i) && vector.Distance(GetPosition(), players.Get(i).GetPosition()) <= s.CleanupRadius) 
			{
				nearby = true; 
				break;
			}
		}
		
		if (!nearby) 
		{
			m_M4DElapsedMs += M4D_CHECK_INTERVAL;
			if (m_M4DElapsedMs >= M4D_MAX_CONTAINER_LIFETIME) 
			{
				GetGame().ObjectDelete(this);
			}
		}
	}

	override void OnDoorUnlocked(DoorLockParams params) 
	{
		super.OnDoorUnlocked(params);
		int doorIdx = params.param1;
		m_LockedMask &= ~(1 << doorIdx);
		SetAnimationPhase(string.Format("side%1_lock", doorIdx + 1), 1);

		if (!m_M4DIsUnlocked) 
		{
			m_M4DIsUnlocked = true;
			SetSynchDirty(); 
			
			if (GetGame() && GetGame().IsServer()) 
			{
				M4D_PlaneCrashWorldState.MarkContainerUnlocked(m_M4DOwnerEventID);
				
				array<Object> objs = new array<Object>();
				GetGame().GetObjectsAtPosition(GetPosition(), 10.0, objs, null);
				
				for (int i = 0; i < objs.Count(); i++) 
				{
					M4D_CrashRewardChest chest = M4D_CrashRewardChest.Cast(objs.Get(i));
					if (chest) 
					{
						if (chest.GetOwnerEventID() == m_M4DOwnerEventID) 
						{
							chest.SetInventoryVisibility(true);
							M4D_PlaneCrashLogger.TraceObject(m_M4DOwnerEventID, "Inventario do bau agora visivel para coleta.");
							break;
						}
					}
				}
			}

			if (!GetGame().IsDedicatedServer()) 
			{
				SEffectManager.PlaySound("Land_ContainerLocked_lock_SoundSet", GetPosition());
			}

			if (GetGame().IsServer()) 
			{
				string playerInfo = M4D_GetNearestPlayerLog(GetPosition());
				string unlockedLog = string.Format("\n[CONTAINER UNLOCKED]\nEventID: %1\n%2\nContainerType: %3\nRewardChest: unlocked", m_M4DOwnerEventID, playerInfo, this.GetType());
				M4D_PlaneCrashLogger.EventContainer(m_M4DOwnerEventID, unlockedLog);
				
				M4D_ConsumeCorrectContainerKeyIfEnabled(GetPosition(), m_M4DOwnerEventID, this.GetType());
				
				ref M4D_PlaneCrashSettings settings = M4D_PlaneCrashSettings.Get();
				if (!settings || settings.EnableOpeningAlarm == 1) 
				{
					int playersReached = 0;
					const int M4D_RPC_ALARM_SOUND = 4857393; 
					array<Man> playersForSound = new array<Man>(); 
					GetGame().GetPlayers(playersForSound);
					vector wPos = GetPosition();
					
					for (int pIdx = 0; pIdx < playersForSound.Count(); pIdx++) 
					{
						PlayerBase playerToHear = PlayerBase.Cast(playersForSound.Get(pIdx));
						if (playerToHear)
						{
							if (playerToHear.GetIdentity()) 
							{
								float distance = vector.Distance(playerToHear.GetPosition(), wPos);
								if (distance <= 1500.0) 
								{
									playersReached = playersReached + 1;
									float speedOfSound = 343.0; 
									float delaySeconds = distance / speedOfSound;
									int delayMilliseconds = Math.Round(delaySeconds * 1000);
									GetGame().RPCSingleParam(playerToHear, M4D_RPC_ALARM_SOUND, new Param2<vector, int>(wPos, delayMilliseconds), true, playerToHear.GetIdentity());
								}
							}
						}
					}
					
					m_M4DAlarmActive = true; 
					SetSynchDirty();

					string alarmLog = string.Format("\n[ALARM DISPATCHED]\nEventID: %1\nRadius: 1500m\nPlayersReached: %2", m_M4DOwnerEventID, playersReached);
					M4D_PlaneCrashLogger.EventContainer(m_M4DOwnerEventID, alarmLog);
				}
			}
		}
	}

	override void OnVariablesSynchronized() 
	{
		super.OnVariablesSynchronized();
		
		if (GetAnimationPhase("Doors1") > 0 || GetAnimationPhase("Doors2") > 0) 
		{
			// Estrutura retida por regra de integridade. Alarme movido para OnDoorUnlocked.
		}
	}

	override bool CanReleaseAttachment(EntityAI attachment) { return false; }
	override bool CanReceiveAttachment(EntityAI attachment, int slotId) { return true; }
	override bool IsInventoryVisible() { return !GetInventory().IsInventoryLockedForLockType(LOCK_FROM_SCRIPT); }
}

	// ---------------------------------------------------------------------------------
	// CLASSE BASE DE ARMAZENAMENTO (M4DCrashStorage)
	// ---------------------------------------------------------------------------------
	// Entidade base para caixas de loot secundário. Nasce com inventário acessível 
	// para retirada, mas utiliza a trava CanReceiveItemIntoCargo para impedir que 
	// jogadores insiram seus próprios itens.
	// ---------------------------------------------------------------------------------
class M4DCrashStorage extends SeaChest 
{
	protected int m_M4DOwnerEventID = -1;
	protected bool m_M4D_IsSystemSpawningLoot = false;
	
	void SetOwnerEventID(int id) { m_M4DOwnerEventID = id; }
	int  GetOwnerEventID()       { return m_M4DOwnerEventID; }

	void SetSystemSpawningMode(bool state) { m_M4D_IsSystemSpawningLoot = state; }

	override bool CanReceiveItemIntoCargo(EntityAI item) { return m_M4D_IsSystemSpawningLoot; }
	
	override bool CanPutInCargo(EntityAI parent) { return false; }
	override bool CanPutIntoHands(EntityAI parent) { return false; }

	override void OnStoreSave(ParamsWriteContext ctx) 
	{
		super.OnStoreSave(ctx);
		ctx.Write(m_M4DOwnerEventID);
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
		return true;
	}
}

	// ---------------------------------------------------------------------------------
	// CLASSE DE ARMAZENAMENTO MILITAR (M4DCrashStorage_MilitaryBox)
	// ---------------------------------------------------------------------------------
class M4DCrashStorage_MilitaryBox extends M4DCrashStorage {}
