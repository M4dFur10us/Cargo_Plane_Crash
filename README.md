# Cargo_Plane_Crash
Sistema avançado de eventos dinâmicos de queda de avião para DayZ, com persistência real, controle por estado, loot por tier e auditoria completa.

🇧🇷 Português
📌 Visão geral

O M4D AirPlane Crash Event transforma eventos de crash em entidades persistentes dentro do servidor.

Cada evento possui:

ID único (EventID)
ciclo de vida completo
estado rastreável
recuperação após restart
validação de integridade em tempo real

Este mod não é um simples sistema de spawn.
Ele foi projetado como uma arquitetura orientada a estado.

⚙️ Como funciona

O sistema NÃO utiliza intervalo tradicional de spawn.

Ele funciona como um modelo de:

Reposição automática de eventos ativos

O servidor tenta manter um número alvo de eventos simultâneos, definido por:

MaxActivePlaneEvents

Cada evento permanece ativo por aproximadamente 30 minutos.

🎯 Lógica prática
Configuração	Resultado
1 evento	Muito raro
3–5 eventos	Frequência média
10+ eventos	Alta atividade

A frequência real depende de:

quantidade de locais configurados
localização dos eventos no mapa
presença de jogadores
conflitos com outros eventos
🧠 Arquitetura
🔹 Entidade Âncora

Responsável por representar o evento no mundo e manter o estado:

fuselagem do avião
dados de runtime
ownership de todos os objetos

🔹 WorldState (Painel Vivo)
Sistema em memória que:

monitora todos os eventos
valida consistência
detecta erros
suporta reidratação

🔹 Reidratação
Após restart:

eventos são restaurados
objetos são re-associados
estado é reconstruído automaticamente

📦 Sistema de recompensa
Cada evento contém:

container trancado
chave única (dropada por zumbi)
baú de recompensa oculto

Fluxo:

Evento nasce
Zumbi recebe a chave
Jogador elimina o zumbi
Container é aberto
Baú é liberado
Loot disponível

🎯 Loot
Gerenciado por:

M4D_PlaneCrashLootManager

Recursos:

loot por tier (1–3)
probabilidade por item
suporte a attachments
suporte a cargo interno
validação de integridade

☣️ Ameaças
zumbis com cota controlada
sistema anti-farming de chave
adaptação automática para zonas de gás
suporte a lobos e ursos

🔊 Sistema de áudio
som da queda do avião
alarme ao abrir container
propagação realista baseada na distância

💾 Persistência
salvamento completo do estado
recuperação automática após restart
reconstrução de eventos

📊 Validação de integridade
O sistema detecta automaticamente:

eventos órfãos
inconsistências de estado
falhas de reidratação
ausência de heartbeat

🧾 Logs

logs por evento
logs de sistema
rastreamento de jogador
auditoria completa
limpeza automática

🧹 Garbage Collector
Executado no boot do servidor:

limpa eventos inválidos
remove objetos órfãos
garante consistência global

🛠️ Configuração
Arquivos:

$profile/M4D_AirPlaneCrash/
PlaneCrashSettings.json
PlaneCrashSites.json
PlaneCrashLoot.json

📍 Locais de crash
Cada local pode definir:

posição
mensagem personalizada
tier
tipos de zumbis
spawn de animais
presença de gás

⚠️ Nota para administradores
A frequência dos eventos depende de:

MaxActivePlaneEvents
locais configurados
densidade de jogadores

🔒 Licença
Este projeto NÃO é open source.

Todos os direitos reservados.
Veja o arquivo LICENSE.txt para detalhes.

📬 Feedback

Sugestões e relatórios de bug são bem-vindos.

🇺🇸 English

📌 Overview

M4D AirPlane Crash Event turns crash events into persistent entities within the server.

Each event has:

unique ID (EventID)
full lifecycle
trackable state
restart recovery
real-time integrity validation

This is not a simple spawn system.
It is designed as a state-driven architecture.

⚙️ How it works

This system does NOT use traditional spawn intervals.

Instead, it uses:

Active event replenishment model

The server maintains a target number of active events defined by:

MaxActivePlaneEvents

Each event lasts approximately 30 minutes.

🎯 Practical logic
Setting	Result
1 event	Very rare
3–5 events	Medium frequency
10+ events	High activity

Actual frequency depends on:

number of configured sites
map positioning
player density
event conflicts
🧠 Architecture
🔹 Anchor Entity

Handles:

aircraft wreck
runtime state
ownership of all related objects
🔹 WorldState (Live Panel)

In-memory system that:

tracks all events
validates integrity
detects inconsistencies
supports rehydration
🔹 Rehydration

After server restart:

events are restored
objects are reassociated
state is rebuilt automatically
📦 Reward system

Each event includes:

locked container
unique key (dropped by zombie)
hidden reward chest

Flow:

Event spawns
Zombie receives key
Player kills zombie
Container unlocked
Chest becomes visible
Loot available
🎯 Loot

Handled by:

M4D_PlaneCrashLootManager

Features:

tier-based loot (1–3)
per-item probability
attachment support
internal cargo support
integrity validation
☣️ Threat system
controlled zombie count
anti-farming key logic
gas-aware behavior (NBC zombies)
wolves and bears support
🔊 Audio system
crash sound
container alarm
distance-based propagation
💾 Persistence
full state saving
restart recovery
automatic event reconstruction
📊 Integrity validation

Automatically detects:

orphan events
state inconsistencies
rehydration failures
missing heartbeat
🧾 Logging
per-event logs
system logs
player tracking
full audit trail
automatic cleanup
🧹 Garbage Collector

Executed on server boot:

removes invalid events
cleans orphan objects
ensures global consistency
🛠️ Configuration

Files:

$profile/M4D_AirPlaneCrash/
PlaneCrashSettings.json
PlaneCrashSites.json
PlaneCrashLoot.json
📍 Crash sites

Each site supports:

position
custom message
tier
zombie types
animal spawn
gas zones
⚠️ Admin note

Event frequency depends on:

MaxActivePlaneEvents
configured locations
player density
🔒 License

This project is NOT open source.

All rights reserved.
See LICENSE.txt for details.

📬 Feedback

Feedback and bug reports are welcome.
