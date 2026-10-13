# LMI — Simulateur de Propagation de Désinformation

> Projet de recherche réalisé dans le cadre du Projet Pi² à l'ESILV en collaboration avec le Campus Cyber pour la **Lutte contre la Manipulation de l'Information (LMI)**  
> Sujet : *Étude du phénomène d'adhésion à une information*

---

## Contexte

La manipulation de l'information (ingénierie sociale de masse) s'appuie sur des facteurs
cognitifs, sociaux et médiatiques pour contourner l'esprit critique. Comprendre le
**phénomène d'adhésion** — pourquoi un individu adopte et relaie une information indépendamment
de sa vérité — est un prérequis pour concevoir des contre-mesures efficaces.

Ce projet répond aux deux axes du sujet :

1. **Identifier les paramètres** (cognitifs, idéologiques, sociaux, contextuels) qui déclenchent
   l'adhésion à une information.
2. **Proposer un modèle** algorithmique et multi-agent pour simuler et prédire l'impact d'une
   campagne de désinformation sur une population cible, et identifier les leviers de résilience.

---

## Approche : Automate Cellulaire sur ESP32

L'implémentation choisie est un **automate cellulaire 25×25** (625 agents) tournant sur
microcontrôleur **ESP32**, avec un tableau de bord web temps réel servi en local via Wi-Fi.

Chaque cellule représente un individu avec un profil cognitif complet. À chaque tick (100ms),
les agents évaluent leur exposition à la désinformation propagée par leurs voisins et décident
probabilistiquement d'y adhérer ou de résister.

---

## Démo rapide

1. Flasher l'ESP32 (voir section Installation)
2. Se connecter au réseau Wi-Fi `ESP32SimulationOCC`
3. Ouvrir `http://192.168.4.1` dans un navigateur
4. Choisir un **récit** (Camp B Extrême ou Camp A Institution)
5. Choisir un **mode de déploiement** (Patient Zéro ou Ferme de Bots)
6. Observer la propagation en temps réel sur la grille
7. Monter le **Stress Ambiant** et observer l'accélération de la contagion

---

## Interface Web

### Page Simulation (`/`)

```
┌─────────────────────────────────────┐
│  Stress Ambiant : [====------] 0.40 │  ← Slider principal
├──────────┬──────────────┬───────────┤
│  Camp A  │ Total Infectés│  Camp B  │  ← Tableau de bord live
│  312 sains│   47 / 625  │ 266 sains │
│  12 infect│             │  35 infect│
├──────────┴──────────────┴───────────┤
│                                     │
│        [Grille 25×25 animée]        │  ← Canvas mis à jour toutes les 250ms
│   🟢 A Sain  🔵 B Sain             │
│   🟤 A Infecté  🔷 B Infecté       │
│                                     │
├─────────────────────────────────────┤
│ 1. Récit de la Fake News            │
│  [Cibler Camp B] [Cibler Camp A]    │  ← Choisir la cible idéologique du message
├─────────────────────────────────────┤
│ 2. Mode de Déploiement              │
│  [Patient Zéro] [Ferme de Bots]     │  ← Choisir le vecteur d'infection initial
│                          [↻ Reset]  │
└─────────────────────────────────────┘
```

### Contrôles détaillés

| Contrôle | Description | Effet sur la simulation |
|---|---|---|
| **Slider Stress Ambiant** | Contexte de crise (0.0 = calme, 1.0 = panique) | > 0.7 : inhibe le raisonnement rationnel. > 0.95 : infection déterministe |
| **Cibler Camp B (Extrême)** | Le message prêche les convictions du Camp B | Le Camp B adhère facilement, le Camp A résiste fortement |
| **Cibler Camp A (Institution)** | Le message s'adresse aux modérés institutionnels | Propagation plus lente mais touche les deux camps |
| **Patient Zéro** | 1 agent infecté au centre de la grille | Propagation organique en vague concentrique |
| **Ferme de Bots** | 6 agents infectés aléatoirement | Propagation multi-foyers, sature plus vite |
| **↻ Recommencer** | Réinitialise la grille avec le mode actuel | Repart de zéro avec un nouveau tirage aléatoire |

### Page Théorie (`/theorie`)

Accessible via le menu de navigation — explique les modèles mathématiques implémentés
(Deffuant-Weisbuch, ELM, formules Logit/Sigmoïde) avec les équations interactives.

### API REST (endpoints)

| Endpoint | Méthode | Auth requise | Description |
|---|---|---|---|
| `/data` | GET | Non | Retourne l'état JSON de la grille (625 états + métriques) |
| `/reset?mode=1` | GET | **Oui** | Réinitialise la grille (mode 1=Patient Zéro, 2=Bots) |
| `/set_stress?value=0.5` | GET | **Oui** | Définit le stress ambiant [0.0 - 1.0] |
| `/set_target?val=1.0` | GET | **Oui** | Définit la cible du message (0.0=Camp A, 1.0=Camp B) |

Authentification via header `X-Token: <token>` ou query param `&token=<token>`.

---

## Lire la grille

Chaque cellule de la grille 25×25 a une couleur selon son état :

| Couleur | État | Signification |
|---|---|---|
| 🟢 Vert clair `#75c181` | Camp A Sain | Agent modéré institutionnel — n'a pas adhéré |
| 🔵 Bleu clair `#61a7d6` | Camp B Sain | Agent sceptique polarisé — n'a pas adhéré |
| 🟤 Vert foncé `#1e5e2b` | Camp A Infecté | Agent Camp A ayant adhéré et relayant le message |
| 🔷 Bleu foncé `#194469` | Camp B Infecté | Agent Camp B ayant adhéré et relayant le message |

Un agent infecté **ne peut pas guérir** dans ce modèle (contagion irréversible, comme un partage viral).

---

## Expériences suggérées

### Exp. 1 — Effet du stress
1. Mode Patient Zéro, message Camp B, stress = 0.0 → observer la propagation lente
2. Monter le stress à 0.8 mi-simulation → observer l'accélération brutale
3. Conclusion : le stress est le levier d'amplification le plus puissant

### Exp. 2 — Ciblage idéologique
1. Lancer avec message "Cibler Camp B" → la propagation reste majoritairement dans le Camp B
2. Relancer avec message "Cibler Camp A" → observer la résistance du Camp A (trust élevé)
3. Conclusion : un message aligné avec les convictions du camp cible est exponentiellement plus efficace

### Exp. 3 — Bot Farm vs Patient Zéro
1. Patient Zéro, stress = 0.5 → noter le temps pour atteindre 50% d'infection
2. Ferme de Bots, stress = 0.5 → comparer le temps
3. Conclusion : la dispersion géographique initiale des bots contourne la résistance de voisinage

---

## Modèles Théoriques Implémentés

### 1. Dynamique des Opinions — Deffuant-Weisbuch (Bounded Confidence)

Un agent ne met à jour sa croyance que si le message est suffisamment proche de son opinion
initiale (tolérance bornée par la confiance institutionnelle et le stress ambiant) :

```
Tolerance  = Trust × (1.0 - Stress)
Si |Opinion - Message| < Tolerance :
    Opinion += (1 - Discernement) × Différence
```

### 2. Passage à l'Acte — ELM + Contagion Complexe (Logit/Sigmoïde)

Inspiré de l'*Elaboration Likelihood Model* (Petty & Cacioppo, 1986) et des modèles de
contagion complexe. Le stress agit comme commutateur entre traitement rationnel et pression
sociale :

```
Défense         = (Discernement + Inoculation) × (1.0 - Stress)
Pulsion_Idéo    = Polarisation × (1.0 - |Opinion - Message|)
Pulsion_Sociale = (Voisins_Infectés / Total_Voisins) × (1.0 + Stress)

Logit           = Pulsion_Idéo + Pulsion_Sociale - Défense - Trust
Probabilité     = sigmoid(5 × Logit) = 1 / (1 + e^(-5 × Logit))
```

---

## Profil d'un Agent

| Paramètre | Description | Rôle |
|---|---|---|
| `opinion` | Position idéologique [0.0=Camp A, 1.0=Camp B] | Détermine la réceptivité au message |
| `trust` | Confiance institutionnelle | Résistance passive |
| `pol` | Polarisation / biais de confirmation | Amplifie l'adhésion si le message est aligné |
| `disc` | Discernement | Défense rationnelle |
| `ino` | Inoculation cognitive | Immunité idéologique acquise |
| `base_camp` | Camp d'appartenance (permanent) | Conservé après infection |
| `state` | 0=A Sain, 1=B Sain, 2=A Infecté, 3=B Infecté | État courant |

---

## Modes de Simulation

| Mode | Description |
|---|---|
| **Patient Zéro** | Un seul agent infecté au centre de la grille — propagation organique |
| **Ferme de Bots** | 6 agents infectés aléatoires simultanément — simulation d'une campagne coordonnée |

---

## Architecture Technique

```
Pisquare-sim/
├── src/
│   ├── main.cpp          ← setup(), loop(), FreeRTOS task launcher
│   ├── simulation.cpp    ← logique automate cellulaire
│   ├── simulation.h
│   ├── web_server.cpp    ← handlers HTTP, auth, rate limiting
│   └── web_server.h
├── data/
│   ├── index.html        ← Dashboard simulation (canvas + sliders)
│   └── theorie.html      ← Page théorie mathématique
├── include/
│   ├── secrets.h         ← (gitignored) credentials locaux
│   └── secrets.h.example ← template à copier
└── platformio.ini
```

### Points techniques notables

- **Dual-core FreeRTOS** : simulation sur Core 0, serveur HTTP sur Core 1 — pas de blocage mutuel
- **Mutex sur la grille** : accès concurrent protégé par `SemaphoreHandle_t`
- **LittleFS** : HTML servi depuis le filesystem flash, séparé du firmware
- **Hardware RNG** : `esp_random()` pour un seed non-déterministe à chaque boot
- **Rate limiting** : endpoint `/data` limité à 10 req/s côté serveur

---

## Stack

- **Matériel** : ESP32 (240MHz dual-core, 320KB RAM, 4MB Flash)
- **Framework** : Arduino / PlatformIO
- **Libs** : ArduinoJson v7, WebServer, LittleFS, FreeRTOS (natif ESP-IDF)
- **Frontend** : HTML5 Canvas + Vanilla JS (aucune dépendance externe)

---

## Installation

### Prérequis
- [PlatformIO](https://platformio.org/) (VSCode extension ou CLI)
- ESP32 connecté en USB

### Configuration

```bash
cp include/secrets.h.example include/secrets.h
# Éditer secrets.h avec vos valeurs
```

```cpp
// include/secrets.h
#pragma once
#define WIFI_SSID      "votre_ssid"
#define WIFI_PASSWORD  "votre_password"
#define API_TOKEN      "votre_token_secret"
```

### Flash

```bash
# 1. Flash le filesystem (HTML)
pio run --target uploadfs

# 2. Flash le firmware
pio run --target upload

# 3. Moniteur série (debug)
pio device monitor
```

Connectez-vous au réseau Wi-Fi de l'ESP32, puis ouvrez `http://192.168.4.1` dans un navigateur.

### Authentification des endpoints de contrôle

```bash
# Via header HTTP
curl -H "X-Token: votre_token" "http://192.168.4.1/reset?mode=1"

# Via query param (navigateur)
http://192.168.4.1/reset?mode=1&token=votre_token
```

---

## Threat Model

| Vecteur d'attaque | Risque | Mitigation appliquée |
|---|---|---|
| Lecture de `/data` sans auth | Exposition de l'état de la grille | Acceptable — aucune donnée sensible |
| Appel non autorisé aux endpoints de contrôle | Reset ou modification des paramètres | Token `X-Token` obligatoire |
| Absence de HTTPS | Token visible en clair sur le réseau local | Acceptable pour démo sur AP isolé |
| DoS par polling `/data` | Saturation du stack TCP ESP32 | Rate limiting 100ms côté serveur |
| Seed RNG déterministe | Simulation identique à chaque reboot | `esp_random()` (hardware TRNG) |

---

## Résultats & Observations

- Un **stress ambiant élevé** (> 0.7) accélère dramatiquement la propagation même avec des
  agents à fort discernement — confirme les modèles de panique informationnelle
- Le mode **Ferme de Bots** sature systématiquement les deux camps plus vite que Patient Zéro,
  particulièrement quand le message cible le camp idéologiquement aligné
- La **polarisation** est le facteur le plus déterminant pour l'adhésion inter-camp :
  un agent très polarisé résiste aux messages opposés mais est hyper-vulnérable aux messages alignés

---

## Références

- Deffuant, G. et al. (2000). *Mixing beliefs among interacting agents.* Advances in Complex Systems.
- Petty, R. E., & Cacioppo, J. T. (1986). *The Elaboration Likelihood Model of Persuasion.*
- Watts, D. J. (2002). *A simple model of global cascades on random networks.* PNAS.
- Centola, D. (2010). *The spread of behavior in an online social network experiment.* Science.
