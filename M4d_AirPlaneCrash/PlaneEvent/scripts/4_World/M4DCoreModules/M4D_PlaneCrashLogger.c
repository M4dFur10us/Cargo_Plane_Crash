// =====================================================================================
// M4D_PlaneCrashLogger.c - VERSÃO FINAL CONSOLIDADA (Fiação Completa e Kill Switch)
// Responsabilidade: Gerir o I/O em disco focado no log administrativo por evento e no
// log de sistema global para falhas de boot e varreduras do MissionServer.
// =====================================================================================

	// ---------------------------------------------------------------------------------
	// CLASSE MESTRA DO LOG (M4D_PlaneCrashLogger)
	// ---------------------------------------------------------------------------------
	// Gerenciador estático absoluto. Avalia dinamicamente a validade das pastas base
	// no sistema de arquivos do servidor. Separa cirurgicamente os logs de inicialização 
	// (System) dos relatórios operacionais individuais de cada acidente aéreo (Events),
	// e realiza a auto-limpeza de arquivos obsoletos (retenção de 14 dias) no boot.
	// ---------------------------------------------------------------------------------
class M4D_PlaneCrashLogger
{
	static int s_LastCheckMs = 0;
	
	static bool s_DebugEnabled = false;
	static bool s_RPTEnabled = true;
	
	static bool s_EventEnabled = true;
	static bool s_EventLogPerEvent = true;
	static bool s_DebugLogPerEvent = true;

	static bool s_LogPlayerInteractions = true;
	static bool s_LogAssetCoordinates = true;
	static bool s_LogThreatDetails = true;
	static bool s_LogSoundDispatch = false;
	static bool s_LogWorldStateSnapshots = true;

	static bool s_CleanupDone = false;
	static const int M4D_LOG_RETENTION_DAYS = 14;

	// ---------------------------------------------------------------------------------
	// ATUALIZADOR DE ESTADO (RefreshFlags)
	// ---------------------------------------------------------------------------------
	// Lê as configurações do JSON (PlaneCrashSettings) e atualiza as booleanas de 
	// controle interno. Possui um gatilho de auto-limpeza que ocorre apenas uma vez 
	// por inicialização de servidor.
	// ---------------------------------------------------------------------------------
	static void RefreshFlags()
	{
		if (!GetGame()) 
		{
			return;
		}

		if (s_CleanupDone == false)
		{
			CleanOldLogs();
			s_CleanupDone = true;
		}
		
		int currentMs = GetGame().GetTime();
		
		if (currentMs - s_LastCheckMs < 10000 && s_LastCheckMs != 0) 
		{
			return;
		}
		
		s_LastCheckMs = currentMs;
		
		s_DebugEnabled = false; 
		s_RPTEnabled = true;
		s_EventEnabled = true;
		s_EventLogPerEvent = true;
		s_DebugLogPerEvent = true;

		s_LogPlayerInteractions = true;
		s_LogAssetCoordinates = true;
		s_LogThreatDetails = true;
		s_LogSoundDispatch = false;
		s_LogWorldStateSnapshots = true;
		
		ref M4D_PlaneCrashSettings s = M4D_PlaneCrashSettings.Get();
		if (!s) 
		{
			return;
		}
		
		if (s.EnableDebugLogging == 1) 
		{ 
			s_DebugEnabled = true; 
		} 
		else 
		{ 
			s_DebugEnabled = false; 
		}
		
		if (s.AlsoLogToRPT == 1) 
		{ 
			s_RPTEnabled = true; 
		} 
		else 
		{ 
			s_RPTEnabled = false; 
		}
		
		if (s.EnableEventLogging == 1) 
		{ 
			s_EventEnabled = true; 
		} 
		else 
		{ 
			s_EventEnabled = false; 
		}
		
		if (s.EnableEventLogPerEvent == 1) 
		{ 
			s_EventLogPerEvent = true; 
		} 
		else 
		{ 
			s_EventLogPerEvent = false; 
		}
		
		if (s.EnableDebugLogPerEvent == 1) 
		{ 
			s_DebugLogPerEvent = true; 
		} 
		else 
		{ 
			s_DebugLogPerEvent = false; 
		}

		if (s.LogPlayerInteractions == 1) 
		{ 
			s_LogPlayerInteractions = true; 
		} 
		else 
		{ 
			s_LogPlayerInteractions = false; 
		}

		if (s.LogAssetCoordinates == 1) 
		{ 
			s_LogAssetCoordinates = true; 
		} 
		else 
		{ 
			s_LogAssetCoordinates = false; 
		}

		if (s.LogThreatDetails == 1) 
		{ 
			s_LogThreatDetails = true; 
		} 
		else 
		{ 
			s_LogThreatDetails = false; 
		}

		if (s.LogSoundDispatch == 1) 
		{ 
			s_LogSoundDispatch = true; 
		} 
		else 
		{ 
			s_LogSoundDispatch = false; 
		}

		if (s.LogWorldStateSnapshots == 1) 
		{ 
			s_LogWorldStateSnapshots = true; 
		} 
		else 
		{ 
			s_LogWorldStateSnapshots = false; 
		}
	}

	// ---------------------------------------------------------------------------------
	// MOTOR DE LIMPEZA DE DISCO (CleanOldLogs)
	// ---------------------------------------------------------------------------------
	// Varre os diretórios de logs, identifica o formato de data dos arquivos e apaga
	// fisicamente do servidor qualquer registro que exceda a janela de retenção de dias.
	// ---------------------------------------------------------------------------------
	static void CleanOldLogs()
	{
		EnsureDirectoriesExist();
		
		int curY, curM, curD;
		GetYearMonthDay(curY, curM, curD);
		
		int currentDayIndex = (curY * 372) + (curM * 31) + curD;
		
		array<string> folders = new array<string>();
		folders.Insert("$profile:M4D_AirPlaneCrash/Logs/");
		folders.Insert("$profile:M4D_AirPlaneCrash/Logs/Events/");
		folders.Insert("$profile:M4D_AirPlaneCrash/Logs/Debug/");
		
		for (int i = 0; i < folders.Count(); i++)
		{
			string folderPath = folders.Get(i);
			string fileName;
			int fileAttr;
			
			FindFileHandle findHandle = FindFile(folderPath + "*.log", fileName, fileAttr, 0);
			if (findHandle != 0)
			{
				bool found = true;
				while (found)
				{
					if (!(fileAttr & FileAttr.DIRECTORY))
					{
						string datePart = "";
						
						if (fileName.Length() >= 17 && fileName.Substring(0, 7) == "System_")
						{
							datePart = fileName.Substring(7, 10);
						}
						else 
						{
							if (fileName.Length() >= 10)
							{
								datePart = fileName.Substring(0, 10);
							}
						}
						
						if (datePart.Length() == 10)
						{
							if (datePart.Substring(4, 1) == "-" && datePart.Substring(7, 1) == "-")
							{
								int fY = datePart.Substring(0, 4).ToInt();
								int fM = datePart.Substring(5, 2).ToInt();
								int fD = datePart.Substring(8, 2).ToInt();
								
								int fileDayIndex = (fY * 372) + (fM * 31) + fD;
								
								if ((currentDayIndex - fileDayIndex) > M4D_LOG_RETENTION_DAYS)
								{
									DeleteFile(folderPath + fileName);
								}
							}
						}
					}
					found = FindNextFile(findHandle, fileName, fileAttr);
				}
				CloseFindFile(findHandle);
			}
		}
	}

	// ---------------------------------------------------------------------------------
	// SANITIZADOR DE NOME DO SERVIDOR (GetCleanServerName)
	// ---------------------------------------------------------------------------------
	// Obtém o nome do servidor ativo e remove caracteres inválidos para compor
	// o nome de arquivos de texto de forma segura pelo sistema operacional.
	// ---------------------------------------------------------------------------------
	static string GetCleanServerName()
	{
		string raw = "DayZServer";
		if (GetGame()) 
		{
			string sn = GetGame().GetServerName();
			if (sn != "") 
			{ 
				raw = sn; 
			}
		}

		string clean = "";
		string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-[]()";
		
		for (int i = 0; i < raw.Length(); i++) 
		{
			string chr = raw.Substring(i, 1);
			if (allowed.Contains(chr)) 
			{ 
				clean = clean + chr; 
			} 
			else 
			{ 
				clean = clean + "_"; 
			}
		}
		
		return clean;
	}

	// ---------------------------------------------------------------------------------
	// VERIFICADOR DE ÁRVORE DE DIRETÓRIOS (EnsureDirectoriesExist)
	// ---------------------------------------------------------------------------------
	// Cria recursivamente as pastas essenciais do mod dentro de $profile, se não existirem.
	// ---------------------------------------------------------------------------------
	static void EnsureDirectoriesExist()
	{
		if (!FileExist("$profile:M4D_AirPlaneCrash")) 
		{
			MakeDirectory("$profile:M4D_AirPlaneCrash");
		}
		
		if (!FileExist("$profile:M4D_AirPlaneCrash/Logs")) 
		{
			MakeDirectory("$profile:M4D_AirPlaneCrash/Logs");
		}
		
		if (!FileExist("$profile:M4D_AirPlaneCrash/Logs/Events")) 
		{
			MakeDirectory("$profile:M4D_AirPlaneCrash/Logs/Events");
		}
		
		if (!FileExist("$profile:M4D_AirPlaneCrash/Logs/Debug")) 
		{
			MakeDirectory("$profile:M4D_AirPlaneCrash/Logs/Debug");
		}
	}

	// ---------------------------------------------------------------------------------
	// CONSTRUTORES DE ROTA E NOMECLATURA (GetFilePath)
	// ---------------------------------------------------------------------------------
	// Constróem strings de localização do disco rígido com base na data e no escopo.
	// ---------------------------------------------------------------------------------
	static string GetSystemLogFilePath()
	{
		EnsureDirectoriesExist();
		int y, mo, d;
		GetYearMonthDay(y, mo, d);
		string dateStr = y.ToStringLen(4) + "-" + mo.ToStringLen(2) + "-" + d.ToStringLen(2);
		return string.Format("$profile:M4D_AirPlaneCrash/Logs/System_%1_%2.log", dateStr, GetCleanServerName());
	}

	static string GetEventLogFilePath(int eventId)
	{
		EnsureDirectoriesExist();
		int y, mo, d;
		GetYearMonthDay(y, mo, d);
		string dateStr = y.ToStringLen(4) + "-" + mo.ToStringLen(2) + "-" + d.ToStringLen(2);
		return string.Format("$profile:M4D_AirPlaneCrash/Logs/Events/%1_Event_%2.log", dateStr, eventId);
	}

	static string GetDebugLogFilePath(int eventId)
	{
		EnsureDirectoriesExist();
		int y, mo, d;
		GetYearMonthDay(y, mo, d);
		string dateStr = y.ToStringLen(4) + "-" + mo.ToStringLen(2) + "-" + d.ToStringLen(2);
		return string.Format("$profile:M4D_AirPlaneCrash/Logs/Debug/%1_Debug_%2.log", dateStr, eventId);
	}

	// ---------------------------------------------------------------------------------
	// MOTOR FÍSICO DE ESCRITA NO DISCO (WriteInternal)
	// ---------------------------------------------------------------------------------
	// Abre o arquivo de texto em modo APPEND, insere a string e fecha instantaneamente.
	// Essencial para evitar bloqueios de arquivo (File Lock) e memory leaks no Enforce.
	// Suporta perfeitamente a injeção de "\n" vinda das classes externas.
	// ---------------------------------------------------------------------------------
	static void WriteInternal(string path, string category, string msg)
	{
		FileHandle file = OpenFile(path, FileMode.APPEND);
		if (file == 0) 
		{
			return;
		}
		
		int h, m, s;
		GetHourMinuteSecond(h, m, s);
		string timeStr = h.ToStringLen(2) + ":" + m.ToStringLen(2) + ":" + s.ToStringLen(2);
		
		string line = string.Format("[%1][%2] %3", timeStr, category, msg);
		FPrintln(file, line);
		CloseFile(file);
	}

	// ---------------------------------------------------------------------------------
	// ALOCADOR E ROTEADOR DE MENSAGENS ADMINISTRATIVAS (WriteEvent)
	// ---------------------------------------------------------------------------------
	// Define se o log será salvo no arquivo geral ou em um arquivo isolado do evento.
	// ---------------------------------------------------------------------------------
	static void WriteEvent(int eventId, string category, string msg)
	{
		RefreshFlags();
		
		if (!s_EventEnabled || eventId <= 0) 
		{
			return;
		}
		
		string finalPath = "";
		string finalMsg = msg;

		if (s_EventLogPerEvent) 
		{
			finalPath = GetEventLogFilePath(eventId);
		} 
		else 
		{
			finalPath = GetSystemLogFilePath();
			finalMsg = string.Format("[EventID: %1] %2", eventId, msg);
		}

		WriteInternal(finalPath, category, finalMsg);
		
		if (s_RPTEnabled) 
		{
			Print(string.Format("[M4D_PlaneCrash][EVENT][%1] %2", eventId, msg)); 
		}
	}

	// ---------------------------------------------------------------------------------
	// ALOCADOR E ROTEADOR DE MENSAGENS TÉCNICAS (WriteDebugInternal)
	// ---------------------------------------------------------------------------------
	// Roteia mensagens exclusivas de desenvolvedor para arquivos dedicados de Debug.
	// ---------------------------------------------------------------------------------
	static void WriteDebugInternal(int eventId, string category, string msg)
	{
		RefreshFlags();
		
		if (!s_DebugEnabled || eventId <= 0) 
		{
			return;
		}
		
		string finalPath = "";
		string finalMsg = msg;

		if (s_DebugLogPerEvent) 
		{
			finalPath = GetDebugLogFilePath(eventId);
		} 
		else 
		{
			finalPath = GetSystemLogFilePath();
			finalMsg = string.Format("[EventID: %1] %2", eventId, msg);
		}

		WriteInternal(finalPath, category, finalMsg);
		
		if (s_RPTEnabled) 
		{
			Print(string.Format("[M4D_PlaneCrash][DEBUG][%1] %2", eventId, msg)); 
		}
	}

	// ---------------------------------------------------------------------------------
	// API DE EVENT LOG PADRÃO (Eventos Táticos e Interações)
	// ---------------------------------------------------------------------------------
	// Estas funções conceituais preparam o cabeçalho do bloco (categoria) e repassam 
	// a string multilinhas formatada diretamente para a gravação física.
	// ---------------------------------------------------------------------------------
	static void EventStart(int eventId, string msg)      
	{ 
		WriteEvent(eventId, "EVENT_START", msg); 
	}
	
	static void EventAborted(int eventId, string reason)
	{
		WriteEvent(eventId, "EVENT_ABORTED", reason);
	}
	
	static void EventAsset(int eventId, string msg)      
	{ 
		RefreshFlags();
		if (!s_LogAssetCoordinates) 
		{
			return;
		}
		WriteEvent(eventId, "ASSETS", msg); 
	}
	
	static void EventContainer(int eventId, string msg)  
	{ 
		WriteEvent(eventId, "CONTAINER", msg); 
	}
	
	static void EventThreats(int eventId, string msg)    
	{ 
		RefreshFlags();
		if (!s_LogThreatDetails) 
		{
			return;
		}
		WriteEvent(eventId, "THREATS", msg); 
	}
	
	static void EventPlayer(int eventId, string msg)     
	{ 
		RefreshFlags();
		if (!s_LogPlayerInteractions) 
		{
			return;
		}
		WriteEvent(eventId, "PLAYER_ACTION", msg); 
	}
	
	static void EventClose(int eventId, string msg)      
	{ 
		WriteEvent(eventId, "EVENT_CLOSED", msg); 
	}

	// ---------------------------------------------------------------------------------
	// API DE DEBUG TÉCNICO (Rastreamento de Funções e Falhas de Execução)
	// ---------------------------------------------------------------------------------
	static void Trace(int eventId, string msg)           { WriteDebugInternal(eventId, "TRACE", msg); }
	static void TraceSpawn(int eventId, string msg)      { WriteDebugInternal(eventId, "SPAWN", msg); }
	static void TraceFunction(int eventId, string msg)   { WriteDebugInternal(eventId, "FUNCTION", msg); }
	static void TraceObject(int eventId, string msg)     { WriteDebugInternal(eventId, "OBJECT", msg); }
	static void TraceFailure(int eventId, string msg)    { WriteDebugInternal(eventId, "FAILURE", msg); }
	static void TraceCallLater(int eventId, string msg)  { WriteDebugInternal(eventId, "CALL_LATER", msg); }
	static void TraceMatrix(int eventId, string msg)     { WriteDebugInternal(eventId, "MATRIX", msg); }
	static void TraceOwnership(int eventId, string msg)  { WriteDebugInternal(eventId, "OWNERSHIP", msg); }

	// ---------------------------------------------------------------------------------
	// API GLOBAL DE SISTEMA (Informativos de Boot e Erros Críticos do Json)
	// ---------------------------------------------------------------------------------
	static void Info(string msg)  
	{ 
		RefreshFlags();
		WriteInternal(GetSystemLogFilePath(), "INFO", msg);
		
		if (s_RPTEnabled) 
		{
			Print("[M4D_PlaneCrash] [INFO] " + msg); 
		}
	}
	
	static void Warn(string msg)  
	{ 
		RefreshFlags();
		WriteInternal(GetSystemLogFilePath(), "WARN", msg);
		
		if (s_RPTEnabled) 
		{
			Print("[M4D_PlaneCrash] [WARN] " + msg); 
		}
	}
	
	static void Error(string msg) 
	{ 
		RefreshFlags();
		WriteInternal(GetSystemLogFilePath(), "ERROR", msg);
		
		if (s_RPTEnabled) 
		{
			Print("[M4D_PlaneCrash] [ERROR] " + msg); 
		}
	}
	
	static void Debug(string msg) 
	{ 
		RefreshFlags();
		if (s_DebugEnabled) 
		{
			WriteInternal(GetSystemLogFilePath(), "DEBUG", msg);
		}
		
		if (s_DebugEnabled && s_RPTEnabled) 
		{
			Print("[M4D_PlaneCrash] [DEBUG] " + msg); 
		}
	}
}