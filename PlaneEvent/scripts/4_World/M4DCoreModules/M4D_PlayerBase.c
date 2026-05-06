// =====================================================================================
// M4D_PlayerBase.c - VERSÃO DEFINITIVA (Múltiplos Canais de Som Ambiental)
// Responsabilidade: Receber os metadados de propagação do servidor e agendar a execução
// do áudio usando a função de ambiente nativa da engine para evitar Memory Kill.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DO JOGADOR (PlayerBase Modificado)
	// ---------------------------------------------------------------------------------
	// Extensão do comportamento base do jogador no cliente para escutar os pacotes
	// de som do evento, validar os dados e acionar as ondas sonoras.
	// ---------------------------------------------------------------------------------
modded class PlayerBase
{
	// Identificadores únicos dos canais RPC definidos no Spawner e Contentores
	static const int M4D_RPC_CRASH_SOUND = 4857392;
	static const int M4D_RPC_ALARM_SOUND = 4857393;

	// ---------------------------------------------------------------------------------
	// GATILHO DE REDE (OnRPC)
	// ---------------------------------------------------------------------------------
	// Acionado automaticamente pela engine quando um pacote de rede (RPC) chega ao 
	// computador do jogador. Ele intercepta e lê o canal específico do nosso mod.
	// ---------------------------------------------------------------------------------
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
		
		// Verifica se estamos rodando no lado do Cliente (Player), onde há placa de som
		if (!GetGame().IsDedicatedServer()) 
		{
			// Canal 1: Som da Queda do Avião
			if (rpc_type == M4D_RPC_CRASH_SOUND) 
			{
				// Prevenção de Null Pointer: Alocação física na memória antes da leitura.
				Param2<vector, int> dataCrash = new Param2<vector, int>(vector.Zero, 0);
				
				if (ctx.Read(dataCrash)) 
				{
					vector crashPosition = dataCrash.param1;
					int delayMs = dataCrash.param2;
					
					// BLOQUEIO DO SPAM DE LOG: Executado apenas em Modo Diagnóstico.
					#ifdef DIAG_DEVELOPER
					Print(string.Format("[M4D_SoundDebug] RPC de Queda Recebido. Agendando som para %1 ms.", delayMs));
					#endif
					
					// O cliente agenda a execução da onda sonora com o atraso correto.
					GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_PlayDelayedWave, delayMs, false, crashPosition);
				}
			}
			// Canal 2: Som do Alarme de Destrancamento
			else if (rpc_type == M4D_RPC_ALARM_SOUND)
			{
				// Prevenção de Null Pointer estrita, mesmo processo do canal primário
				Param2<vector, int> dataAlarm = new Param2<vector, int>(vector.Zero, 0);
				
				if (ctx.Read(dataAlarm)) 
				{
					vector alarmPosition = dataAlarm.param1;
					int alarmDelayMs = dataAlarm.param2;
					
					// Rastreio diagnóstico isolado para o alarme
					#ifdef DIAG_DEVELOPER
					Print(string.Format("[M4D_SoundDebug] RPC de Alarme Recebido. Agendando som para %1 ms.", alarmDelayMs));
					#endif
					
					// Agenda o som do alarme garantindo a distância de propagação baseada na velocidade do som
					GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.M4D_PlayDelayedAlarmWave, alarmDelayMs, false, alarmPosition);
				}
			}
		}
	}

	// ---------------------------------------------------------------------------------
	// EXECUÇÃO DO ÁUDIO DO AVIÃO (M4D_PlayDelayedWave)
	// ---------------------------------------------------------------------------------
	// Disparada pelo CallQueue. Agora utilizamos PlaySoundEnviroment, que é a função
	// recomendada pela Bohemia para ataques de artilharia e eventos gigantes distantes.
	// ---------------------------------------------------------------------------------
	void M4D_PlayDelayedWave(vector pos)
	{
		#ifdef DIAG_DEVELOPER
		Print(string.Format("[M4D_SoundDebug] EXECUTANDO AUDIO DA QUEDA AGORA na posicao: %1", pos.ToString()));
		#endif
		
		// CORREÇÃO: Disparo seguro pelo ambiente. Como o config.cpp agora tem a curva
		// e o SoundShader corretos, a engine propagará isso no mapa sem matar a memória.
		SEffectManager.PlaySoundEnviroment("PlaneCrash_SoundSet", pos);
	}

	// ---------------------------------------------------------------------------------
	// EXECUÇÃO DO ÁUDIO DO ALARME (M4D_PlayDelayedAlarmWave)
	// ---------------------------------------------------------------------------------
	// Extensão idêntica ao som do avião, mas evocando o SoundSet correto do alarme.
	// Utiliza PlaySoundEnviroment para respeitar a distância e atenuar bugs de memória.
	// ---------------------------------------------------------------------------------
	void M4D_PlayDelayedAlarmWave(vector pos)
	{
		#ifdef DIAG_DEVELOPER
		Print(string.Format("[M4D_SoundDebug] EXECUTANDO AUDIO DO ALARME AGORA na posicao: %1", pos.ToString()));
		#endif
		
		// Dispara a malha de som do alarme, chamando o SoundSet modificado no config.cpp
		SEffectManager.PlaySoundEnviroment("PlaneCrash_UnlockAlarm_SoundSet", pos);
	}
}