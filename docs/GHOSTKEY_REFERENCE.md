# GhostKey — Beeps, Lockouts, Holds & Sequences

Reference derived from `src/GhostKey.ino` (verify against `#define` values after edits).

---

## 1. Buzzer / beep catalog

| ID | Pattern | Total time (approx.) | When it fires | Config mode? |
|----|---------|----------------------|---------------|--------------|
| **P0** | 120 ms pulse | ~120 ms | Cold boot / reset only (`ESP_SLEEP_WAKEUP_UNDEFINED`); **not** on ESP deep-sleep wake | N/A |
| **A1** | 100 ms | ~100 ms | First RFID auth after boot (single “key accepted” chirp) | No |
| **A2** | 200 ms + 100 ms gap + 200 ms | ~500 ms | Same authorized tag scanned again after **≥ 2 s** → RFID deauth | No |
| **U1** | 100 ms + 100 ms gap + 100 ms | ~300 ms | Unauthorized tag (non‑zero bytes, fails key check); **once per 5 s** max | No |
| **T1** | 200 ms | ~200 ms | RFID auth **30 s timeout** while system **OFF** (accessory/ignition not active) | No |
| **C1** | Continuous **2 s** tone | 2000 ms | RFID **5 s hold** → enter config (Ghost Power only / setup incomplete paths) | N/A (entry) |
| **C2** | Continuous **1 s** tone | 1000 ms | RFID **5 s hold** → exit config (Ghost Power only) | N/A (exit) |
| **CAL** | 500 ms | ~500 ms | BLE calibration **completed** (web flow) | Via web |

```mermaid
flowchart LR
  subgraph Beeps["Buzzer patterns (duration)"]
    P0["P0: 120ms"]
    A1["A1: 100ms"]
    A2["A2: 200+100+200ms"]
    U1["U1: 100+100+100ms"]
    T1["T1: 200ms"]
    C1["C1: 2000ms hold"]
    C2["C2: 1000ms hold"]
    CAL["CAL: 500ms"]
  end
```

---

## 2. Lockouts, cooldowns & “blocked” windows

| Name | Duration | What it blocks / means |
|------|----------|-------------------------|
| **Post‑engine‑shutdown config lockout** | **5 s** | `enterConfigMode()` (RFID long hold + start‑button long press paths) after GhostKey **software** engine stop |
| **Engine restart cooldown** | **3 s** | New **start sequence** after `lastEngineShutdown` (brake+button start only) |
| **Unauthorized RFID beep cooldown** | **5 s** | Repeat **U1** beep |
| **WiFi / button release guard** | **5 s** | Ignores some **button release** (cycle OFF→ACC→IGN) after WiFi ops |
| **Config mode: tap to exit** | **5 s** since last press | Start button in config must wait **> 5 s** since `lastButtonPress` to exit |
| **Web session** | **10 min** | `WEB_SESSION_TIMEOUT` |
| **Web inactivity** | **5 min** | `WEB_ACTIVITY_TIMEOUT` |
| **Low battery → deep sleep dwell** | **10 min** continuous low band | Voltage **≤ 12.2 V** (start dwell); cleared only **≥ 12.35 V**; **no sleep** if **IGN on** (`ACCESSORY_INPUT_PIN` LOW) or **engineRunning** |
| **Low battery wake grace** | **60 s** | After low‑bat deep sleep wake: full power; then sleep again if still bad and no IGN/engine |
| **Factory reset hold** | **30 s** | Start + brake held together (`FACTORY_RESET_TIME`) |

```mermaid
flowchart TB
  subgraph Lock["Lockouts (typical order of checks)"]
    L1["IGN ON or engine running? → no low‑bat deep sleep"]
    L2["Voltage ≥ 12.35V? → clear low‑bat dwell"]
    L2b["Voltage ≤ 12.2V 10 min? → deep sleep (if L1 false)"]
    L3["Within 5s of engine shutdown? → config blocked"]
    L4["Within 3s of engine shutdown? → start blocked"]
    L5["Within 5s of unauthorized tag? → no second U1 beep"]
  end
  L1 --> L2
  L2 --> L2b
  L2b --> L3
  L3 --> L4
  L4 --> L5
```

---

## 3. Start button + brake — normal driving sequence

**Inputs:** start button **active LOW**, brake **active LOW** when pressed. Debounce **50 ms** on both.

```mermaid
flowchart TB
  subgraph Stable["Brake stable (anti‑noise)"]
    B1["Brake LOW debounced"]
    B2["Wait BRAKE_MIN_HOLD_MS = 400ms"]
    B3["brakeOkForBrakeButtonAction = true"]
    B1 --> B2 --> B3
  end

  subgraph Start["Start (engine not running)"]
    S1["Brake OK + button press edge"]
    S2{"Within 3s of lastEngineShutdown?"}
    S3["Ignore start"]
    S4{"RFID or BLE auth?"}
    S5["Start relay sequence"]
    S6["LED blink error ×3"]
    S1 --> S2
    S2 -->|yes| S3
    S2 -->|no| S4
    S4 -->|no| S6
    S4 -->|yes| S5
  end

  subgraph Stop["Stop engine (software state)"]
    T1["Running + brake + start together → immediate off"]
    T2["Running + NO brake + hold start 3s → off (failsafe)"]
    T3["engineRunning=false, relays updated, lastEngineShutdown=now"]
    T1 --> T3
    T2 --> T3
    T4["Release start before 3s → cancel no-brake shutdown"]
    T2 -.-> T4
  end

  Stable --> Start
  Stable --> Stop
```

**Accessory / ignition stepping (no brake):** button **release** with brake **not** held cycles `systemState`: **OFF → ACC → IGN → OFF** (if authenticated), subject to WiFi **5 s** guard and not while shutting down / cranking / engine running.

---

## 4. Start button — config mode (long press)

| Mode | Brake | Hold time | Auth required | Result |
|------|-------|-----------|---------------|--------|
| **Ghost Key enabled** | **Not** held | **5 s** (`CONFIG_MODE_PRESS_TIME`) | RFID or (BLE + enabled + GK) | `enterConfigMode()` — **only when `!engineRunning`** |
| **Ghost Power only** (`!ghostKeyEnabled && ghostPower`) | **Not** held | **5 s** | RFID or BT auth (path in code) | `enterConfigMode()` |

Additional: **5 s** post‑shutdown **config lockout** (see §2). `enterConfigMode()` also requires auth.

**Exit config:** short press start in config if **`millis() - lastButtonPress > 5000`**.

---

## 5. RFID — auth, deauth, config holds

| Constant | Value | Role |
|----------|-------|------|
| `RFID_AUTH_TIMEOUT` | **30 s** | Auth expires when **system OFF** and not cranking / not in accessory+ / no engine |
| `RFID_DEAUTH_TIMEOUT` | **2 s** | Min time after auth before **same tag** scan counts as **deauth** (A2 beep) |
| `RFID_CONFIG_HOLD_TIME` | **5 s** | Continuous tag presence → config **enter** (eligible modes only) |
| `RFID_CONFIG_EXIT_HOLD_TIME` | **5 s** | Continuous tag → config **exit** (Ghost Power only, in config) |
| `RFID_CONTINUOUS_READ_TIMEOUT` | **2 s** | Max gap between reads; hold timers **reset** if tag “lost” longer than this |

```mermaid
sequenceDiagram
  participant Tag as RFID tag
  participant FW as Firmware

  Tag->>FW: Authorized key read
  FW->>FW: Auth ON (30s timer, reset while car active)
  Note over FW: First auth: A1 beep

  Tag->>FW: Same tag after ≥2s
  FW->>FW: Deauth + A2 beep

  Tag->>FW: Wrong key (non‑zero)
  FW->>FW: U1 double beep (max 1 / 5s)

  Tag->>FW: Held ≤2s gaps for 5s (GP only / setup)
  FW->>FW: C1 2s buzzer → enterConfigMode
```

---

## 6. Cranking / starter timing

| Item | Value |
|------|--------|
| Default starter pulse | `starterPulseTime` from NVS, default **`STARTER_PULSE_TIME` = 700 ms** |
| **Extended crank** | If still **button + brake + auth + startRelayActive** after initial pulse → crank continues until release |
| Relay pattern during crank | IGN1+START on; ACC/IGN2 off (see `controlRelays()`) |
| Engine running | ACC+IGN1+IGN2 on; START off |

---

## 7. Battery & deep sleep (summary)

```mermaid
stateDiagram-v2
  [*] --> Monitoring: Normal run
  Monitoring --> DwellLow: V ≤ 12.2V (IGN off, engine off)
  DwellLow --> Monitoring: V ≥ 12.35V (clear dwell)
  DwellLow --> PreSleep: 10 min elapsed in low band
  Monitoring --> Monitoring: IGN ON or engine → dwell cleared / no sleep
  PreSleep --> DeepSleep: enterUltraLowBatteryDeepSleep
  note right of PreSleep
    If ACC/IGN relays were ON,
    firmware drives them LOW first,
    then deep sleep (EXT1 wake: button+brake).
  end note
```

---

## 8. Other notable timers (BLE / power)

| Name | Value | Notes |
|------|-------|-------|
| `LIGHT_SLEEP_DELAY_MS` | 20 s | After unauthenticated (power path messaging) |
| `BRAKE_WAKEUP_ACTIVE_TIME` | 30 s | Extended active after brake/button wake from light sleep |
| Brake wake stabilize | **2 s** | Before completing brake wake sequence |
| `BLE_DEEP_SLEEP_INTERVAL_MS` | 12 s | Deep sleep BLE cycle total |
| `BLE_DEEP_SLEEP_DURATION_MS` | 8 s | Advertising window in that cycle |
| `CONFIG_MODE_TIMEOUT` | 30 s | Config session timeout (if used in code paths) |
| `AUTO_LOCK_TIMEOUT` | 30 s | Security autolock timing (preferences) |
| `FACTORY_RESET_TIME` | 30 s | Start + brake factory reset |

---

## 9. How to keep this doc accurate

1. Search `src/GhostKey.ino` for `#define` blocks under **TIMING CONSTANTS**, **RFID**, **Battery**, **Brake/start failsafe**.
2. Search for `buzzerPulse`, `buzzerOn`, `enterConfigMode`, `enterUltraLowBatteryDeepSleep`.

---

*Generated as a human‑readable map of behavior; not a substitute for release notes or wiring diagrams.*
