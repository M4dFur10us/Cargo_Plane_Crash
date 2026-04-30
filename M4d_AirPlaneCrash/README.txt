README - M4D AirPlane Crash Event

Mod Name:
M4D AirPlane Crash Event

Description:
Advanced dynamic airplane crash event system for DayZ, built with a focus on stability, real persistence, and consistent behavior within the limitations of the Enforce Engine.

This mod was designed for real server environments, avoiding common issues such as non-persistent events, improper cleanup, and unpredictable behavior after server restarts.

---

MAIN FEATURES

Robust architecture

* The airplane (wreck) is the core entity of the event
* Ownership system based on EventID
* Centralized control via MissionServer

Persistence

* Events survive server restarts
* Rehydrates containers, zombies, animals, gas, and assets

Cleanup system

* Ownership-based cleanup (EventID)
* Safe fallback for vanilla objects
* Automatic cleanup on server boot

Gameplay

* Locked containers with dynamic loot
* Anti key farming system
* Threat spawning (zombies, animals, gas)
* Configurable loot

Audio

* Crash sound with distance-based propagation
* Individual delay per player

WorldState

* Active event tracking
* Automatic auditing
* Lightweight snapshot for diagnostics

---

HOW IT WORKS

Event lifecycle:

1. Server starts
2. Waits approximately 60 seconds for stabilization
3. Event is automatically triggered
4. System manages spawn, threats, loot, and lifecycle
5. Event is automatically cleaned up at the end

---

MASTER SWITCH (ENABLE / DISABLE MOD)

File:
/profile/M4D_PlaneCrashSettings.json

Parameter:
EnableMod

EnableMod = 0

* No new events will be started
* All active events will be automatically cleaned on server boot

EnableMod = 1

* System operates normally
* Events follow configured settings

---

CONFIGURATION

File:
/profile/M4D_PlaneCrashSettings.json

Main parameters:

EnableMod
Enable or disable the system

MaxActivePlaneEvents
Maximum number of simultaneous events

SafeRadius
Minimum distance from players for spawn

DistanceRadius
Minimum distance between events

CleanupRadius
Container cleanup radius

EnableGeneralDebug
Enable general logs

EnableFileDebug
Enable file logging

---

IMPORTANT

DayZ compatibility

This mod was developed considering real Enforce Engine limitations:

* Does NOT use ternary operators
* Does NOT rely on automatic engine persistence
* Does NOT use structures incompatible with the DayZ runtime

Containers

Container classes are intentionally duplicated.
Do NOT attempt to merge or refactor these classes.
Doing so may cause server crashes.

Server restart

The mod is designed to:

* Persist correctly
* Restore events after restart
* Prevent permanent leftover objects on the map

---

PERFORMANCE

This mod was designed for real server usage.

On heavily populated servers, performance impact may occur in:

* Event location selection
* Threat spawning

It is recommended to adjust settings according to server conditions.

---

INSTALLATION

1. Install the mod on the server
2. Start the server once to generate configuration files
3. Edit:
   /profile/M4D_PlaneCrashSettings.json
4. Restart the server

---

LOGS

Location:
/profile/M4D_Logs/

---

LICENSE AND USAGE

This mod follows Bohemia Interactive guidelines for DayZ modding and Steam Workshop policies.

Prohibited:

* Repacking the mod
* Reuploading on any platform
* Redistribution without permission

Allowed:

* Use on public and private servers
* Configuration changes via JSON files

---

CREDITS

Inspired by the Plane Crash mod created by BuffaGunz:
[https://steamcommunity.com/sharedfiles/filedetails/?id=3507617679](https://steamcommunity.com/sharedfiles/filedetails/?id=3507617679)

---

AUTHOR

xXM4dFur10usXx - SteamID:76561198301862284


-------------


README - M4D AirPlane Crash Event - PT/BR

Nome do mod:
AirPlane Crash Event

Descricao:
Sistema avancado de eventos dinamicos de queda de aviao para DayZ, desenvolvido com foco em estabilidade, persistencia real e funcionamento consistente dentro das limitacoes da Enforce Engine.

Este mod foi projetado para operacao em servidores reais, evitando problemas comuns como eventos que nao persistem, limpeza incorreta e comportamento imprevisivel apos restart do servidor.

PRINCIPAIS FUNCIONALIDADES

Arquitetura robusta

O aviao (wreck) e a entidade principal do evento
Sistema de ownership baseado em EventID
Controle centralizado via MissionServer

Persistencia

Eventos sobrevivem a restart do servidor
Reidrata containers, zombies, animais, gas e assets

Sistema de limpeza

Cleanup baseado em ownership (EventID)
Fallback seguro para objetos vanilla
Sistema de limpeza automatica no boot

Gameplay

Containers trancados com loot dinamico
Sistema anti farming de chave
Spawn de threats (zombies, animais, gas)
Loot configuravel

Audio

Som da queda com propagacao por distancia
Delay individual por jogador

WorldState

Controle de eventos ativos
Auditoria automatica
Snapshot leve para diagnostico

FUNCIONAMENTO

Ciclo do evento:

Servidor inicia
Aguarda aproximadamente 60 segundos para estabilizacao
Evento e iniciado automaticamente
Sistema gerencia spawn, threats, loot e lifecycle
Evento e encerrado e limpo automaticamente

MASTER SWITCH (LIGAR E DESLIGAR O MOD)

Arquivo:
/profile/M4D_PlaneCrashSettings.json

Parametro:
EnableMod

EnableMod = 0

Nenhum novo evento sera iniciado
Todos os eventos ativos serao limpos automaticamente no boot

EnableMod = 1

Sistema opera normalmente
Eventos seguem as configuracoes definidas

CONFIGURACAO

Arquivo:
/profile/M4D_PlaneCrashSettings.json

Principais parametros:

EnableMod
Liga ou desliga o sistema

MaxActivePlaneEvents
Numero maximo de eventos simultaneos

SafeRadius
Distancia minima de jogadores para spawn

DistanceRadius
Distancia minima entre eventos

CleanupRadius
Raio de limpeza de containers

EnableGeneralDebug
Ativa logs gerais

EnableFileDebug
Ativa logs em arquivo

IMPORTANTE

Compatibilidade com DayZ

Este mod foi desenvolvido considerando as limitacoes reais da Enforce Engine:

Nao utiliza operador ternario
Nao depende de persistencia automatica da engine
Nao utiliza estruturas incompatíveis com o runtime do DayZ

Containers

As classes de containers sao intencionalmente duplicadas.
Nao tente unificar ou refatorar essas classes.
Isso pode causar falha ou crash no servidor.

Restart do servidor

O mod foi projetado para:

Persistir corretamente
Restaurar eventos apos restart
Evitar lixo permanente no mapa

PERFORMANCE

O mod foi testado para uso em servidores reais.

Em servidores com muitos jogadores, pode haver impacto em:

Escolha de local do evento
Spawn de threats

Recomenda-se ajustar os parametros conforme a realidade do servidor.

INSTALACAO

Instale o mod no servidor
Inicie o servidor uma vez para gerar os arquivos
Edite:
/profile/M4D_PlaneCrashSettings.json
Reinicie o servidor

LOGS

Local:
/profile/M4D_Logs/

LICENCA E USO

Este mod segue as diretrizes da Bohemia Interactive para mods de DayZ e as politicas da Steam Workshop.

Proibido:

Repack do mod
Reupload em qualquer plataforma
Redistribuicao sem autorizacao

Uso permitido:

Uso em servidores publicos e privados
Modificacao de configuracao via arquivos JSON

CREDITOS

Inspirado no mod Plane Crash criado por BuffaGunz:
https://steamcommunity.com/sharedfiles/filedetails/?id=3507617679

AUTOR

xXM4dFur10usXx - SteamID:76561198301862284