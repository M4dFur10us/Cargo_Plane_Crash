# M4D AirPlane Crash Event

### 🇧🇷 Português

Um sistema avançado de eventos dinâmicos de queda de avião para DayZ.

Mais do que um simples spawn de loot, este mod transforma cada crash em um **evento persistente**, com ciclo de vida, progressão, risco real, persistência pós-restart e auditoria administrativa.

---

## ✈️ O que este mod faz

* Gera eventos de crash de avião no mapa
* Cada evento possui uma fuselagem persistente
* Cada evento possui container trancado
* A chave é dropada por zumbis
* O loot principal fica protegido em um baú oculto
* O evento exige interação real do jogador
* O estado do evento pode ser salvo e restaurado após restart do servidor

---

## ⚙️ Como o sistema funciona

Este mod NÃO utiliza intervalo tradicional de spawn.

Ele funciona com um modelo de:

> **Reposição automática de eventos ativos**

O servidor tenta manter um número de eventos simultâneos baseado em:

`MaxActivePlaneEvents`

Se houver espaço para novos eventos ativos, o sistema poderá tentar gerar outro evento rapidamente. Isso é intencional.

O controle de raridade, dificuldade e frequência é feito principalmente por:

* quantidade máxima de eventos ativos
* duração dos eventos
* localização dos crash sites
* presença de jogadores próximos
* ocupação da área por outros eventos

---

## 🎯 Como controlar a frequência

Você controla a dificuldade e frequência via:

* quantidade máxima de eventos ativos
* localização dos crash sites
* distribuição dos sites no mapa
* densidade de jogadores nas regiões configuradas

### Exemplos:

* **1 evento** → raro e difícil
* **3 a 5 eventos** → frequência equilibrada
* **10+ eventos** → alta atividade

Para tornar os eventos mais raros, reduza `MaxActivePlaneEvents` e configure os crash sites em áreas remotas, pouco visitadas ou de difícil acesso.

Para tornar os eventos mais acessíveis, aumente `MaxActivePlaneEvents` e configure os crash sites em áreas centrais, próximas a cidades ou regiões de maior circulação.

---

## 📍 Locais dos eventos

Os locais são totalmente configuráveis via JSON.

Você pode definir:

* onde os eventos acontecem
* mensagem personalizada do evento
* tier do local
* tipos de zumbis
* presença de zona de gás
* suporte a lobos e ursos

Isso permite criar eventos mais fáceis, mais difíceis, mais remotos ou mais disputados, conforme o estilo do servidor.

---

## 🎁 Sistema de recompensa

Cada evento inclui:

* container trancado
* chave única dropada por zumbi
* baú de recompensa oculto
* loot por tier
* loot principal e secundário

Fluxo básico:

1. O crash acontece
2. Zumbis aparecem na área
3. Um zumbi pode carregar a chave
4. O jogador encontra a chave
5. O container é destrancado
6. O baú de recompensa fica visível
7. O loot pode ser coletado

---

## ☣️ Ameaças

O sistema de ameaças pode incluir:

* zumbis com quantidade controlada
* zumbis NBC quando há gás
* zona contaminada
* lobos
* ursos
* sistema anti-farming de chave

A quantidade de zumbis é configurável. Se `ZombieCount` for definido como `0`, o sistema não gerará zumbis e, consequentemente, não haverá chave dropada por zumbi.

---

## 🔊 Imersão

O mod possui sistema de áudio próprio para aumentar a sensação de evento dinâmico.

Inclui:

* som de queda do avião
* alarme ao abrir o container
* propagação sonora baseada na distância
* delay sonoro calculado conforme a distância do jogador

---

## 💾 Persistência

O sistema foi construído para lidar com reinícios de servidor.

Recursos:

* eventos podem continuar após restart
* estado é salvo automaticamente
* objetos são re-associados ao evento
* sistema de recuperação inteligente
* limpeza de entidades órfãs

---

## 📊 Estabilidade

O mod possui mecanismos internos para reduzir eventos quebrados e lixo persistente no servidor.

Inclui:

* validação automática de integridade
* monitoramento de eventos ativos
* prevenção de eventos órfãos
* limpeza de objetos associados
* logs completos para administração
* sistema de WorldState para auditoria

---

## 🧹 Limpeza e gerenciamento de entidades

O mod utiliza um sistema de ownership por evento.

Cada crash possui um `EventID`, usado para associar:

* fuselagem
* container
* baú de recompensa
* assets secundários
* zumbis
* animais
* zonas de gás

Quando um evento é encerrado, o sistema tenta remover as entidades relacionadas para evitar acúmulo indevido no servidor.

---

## 🛠️ Configuração

Arquivos:

```text
$profile/M4D_AirPlaneCrash/
```

Arquivos principais:

* `PlaneCrashSettings.json`
* `PlaneCrashSites.json`
* `PlaneCrashLoot.json`

---

## ⚠️ Notas operacionais conhecidas

### Modelo de spawn

Este mod não usa intervalo tradicional de spawn.

Ele usa um modelo de reposição automática de eventos ativos. O sistema tenta manter a quantidade de eventos definida em `MaxActivePlaneEvents`.

Se houver espaço para novos eventos ativos, o sistema poderá tentar gerar outro evento rapidamente. Isso não é bug.

---

### Duração dos eventos

Cada evento permanece ativo por aproximadamente 30 minutos, desde que não haja jogadores próximos.

Eventos podem permanecer por mais tempo se houver jogadores dentro do raio de atividade definido nas configurações.

---

### Locais rejeitados

Um local de crash pode ser rejeitado se houver:

* jogadores próximos
* outro evento M4D ativo na região
* helicrash nativo próximo
* conflito com objetos existentes
* área ocupada ou inválida

Isso é esperado e ajuda a evitar sobreposição de eventos.

---

### Zumbis e chave

A chave do container depende da configuração de zumbis.

Se `ZombieCount` estiver definido como `0`, o sistema não gerará zumbis e também não haverá chave dropada por zumbi.

---

### Persistência após restart

Eventos ativos podem ser restaurados após restart do servidor.

Durante o boot, o sistema tenta reidratar eventos existentes e limpar objetos órfãos automaticamente.

---

## 🔒 Licença

Este mod NÃO é open source.

Todos os direitos reservados.

Veja o arquivo `LICENSE.txt` para detalhes.

---

## 📬 Feedback

Sugestões, relatórios de bugs e feedbacks de servidores são bem-vindos.

---

### 📌 Direitos Autorais

Copyright (c) 2026 - xXM4dFur10usXx
SteamID: 76561198301862284
Todos os direitos reservados.

---

# 🇺🇸 English

An advanced dynamic airplane crash event system for DayZ.

More than just loot spawning, this mod turns crashes into **persistent, state-driven events** with progression, risk, restart recovery, and administrative auditing.

---

## ✈️ What this mod does

* Spawns airplane crash events across the map
* Each event has a persistent wreck entity
* Each event includes a locked container
* The key is carried by a zombie
* Main loot is stored in a hidden reward chest
* The event requires real player interaction
* Event state can be saved and restored after server restart

---

## ⚙️ How it works

This mod does NOT use traditional spawn intervals.

It operates using:

> **Active event replenishment model**

The server attempts to maintain a number of active events defined by:

`MaxActivePlaneEvents`

If there is room for more active events, the system may try to spawn another event quickly. This is intentional.

Rarity, difficulty, and frequency are mainly controlled by:

* maximum number of active events
* event lifetime
* crash site locations
* nearby player presence
* area occupation by other events

---

## 🎯 Controlling frequency

You control difficulty and frequency through:

* maximum number of active events
* crash site locations
* site distribution across the map
* player density around configured areas

### Examples:

* **1 event** → rare and difficult
* **3 to 5 events** → balanced frequency
* **10+ events** → high activity

To make events rarer, reduce `MaxActivePlaneEvents` and configure crash sites in remote, low-traffic, or hard-to-reach areas.

To make events more accessible, increase `MaxActivePlaneEvents` and configure crash sites near central areas, towns, or high-traffic regions.

---

## 📍 Crash locations

Crash locations are fully configurable through JSON.

Each site can define:

* event position
* custom event message
* site tier
* zombie types
* gas zone presence
* wolves and bears support

This allows server owners to create easier, harder, remote, or contested crash events depending on the server style.

---

## 🎁 Reward system

Each event includes:

* locked container
* unique zombie-dropped key
* hidden reward chest
* tier-based loot
* main and secondary loot

Basic flow:

1. Crash event spawns
2. Zombies spawn in the area
3. One zombie may carry the key
4. Player finds the key
5. Container is unlocked
6. Reward chest becomes visible
7. Loot can be collected

---

## ☣️ Threats

The threat system may include:

* controlled zombie count
* NBC zombies when gas is present
* contaminated gas zone
* wolves
* bears
* anti-farming key logic

Zombie count is configurable. If `ZombieCount` is set to `0`, the system will not spawn zombies and therefore no zombie-carried key will be generated.

---

## 🔊 Immersion

The mod includes its own audio system to improve the dynamic event experience.

Features:

* airplane crash sound
* alarm when opening the container
* distance-based sound propagation
* sound delay calculated by player distance

---

## 💾 Persistence

The system is designed to handle server restarts.

Features:

* events may survive server restarts
* state is saved automatically
* objects are re-associated with the event
* intelligent recovery system
* orphan entity cleanup

---

## 📊 Stability

The mod includes internal mechanisms to reduce broken events and persistent server clutter.

Includes:

* automatic integrity validation
* active event monitoring
* orphan event prevention
* cleanup of associated objects
* full administrative logs
* WorldState system for auditing

---

## 🧹 Cleanup and entity management

The mod uses an event ownership system.

Each crash has an `EventID`, used to associate:

* wreck
* container
* reward chest
* secondary assets
* zombies
* animals
* gas zones

When an event is closed, the system attempts to remove related entities to prevent unnecessary accumulation on the server.

---

## 🛠️ Configuration

Files:

```text
$profile/M4D_AirPlaneCrash/
```

Main files:

* `PlaneCrashSettings.json`
* `PlaneCrashSites.json`
* `PlaneCrashLoot.json`

---

## ⚠️ Known operational notes

### Spawn model

This mod does not use a traditional spawn interval.

It uses an active event replenishment model. The system attempts to maintain the number of events defined by `MaxActivePlaneEvents`.

If there is room for more active events, the system may try to spawn another event quickly. This is not a bug.

---

### Event lifetime

Each event lasts approximately 30 minutes, unless players remain nearby.

Events may stay active longer if players are inside the configured activity radius.

---

### Rejected locations

A crash site may be rejected if there are:

* nearby players
* another active M4D event in the area
* native helicrash nearby
* conflict with existing objects
* occupied or invalid area

This is expected behavior and helps prevent event overlap.

---

### Zombies and keys

The container key depends on zombie configuration.

If `ZombieCount` is set to `0`, the system will not spawn zombies and therefore no zombie-carried key will be generated.

---

### Restart persistence

Active events may be restored after server restart.

On server boot, the system attempts to rehydrate existing events and automatically clean orphaned objects.

---

## 🔒 License

This mod is NOT open source.

All rights reserved.

See `LICENSE.txt` for details.

---

### 📌 Direitos Autorais

Copyright (c) 2026 - xXM4dFur10usXx
SteamID: 76561198301862284
Todos os direitos reservados.

---

## 📬 Feedback

Suggestions, bug reports, and server feedback are welcome.
