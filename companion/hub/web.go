package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"sync"
)

type HubWebState struct {
	mu           sync.RWMutex
	TargetIP     string
	ActiveView   uint8
	Brightness   uint8
	AudioMuted   bool
	NightMode    bool
	PCActive     bool
	CustomNotif  string
	LastAction   string
	ServerStatus ServerMetrics
	Weather      WeatherData
}

var globalWebState = &HubWebState{
	TargetIP:   "192.168.101.115",
	ActiveView: 0,
	Brightness: 80,
	AudioMuted: true,
	NightMode:  false,
	PCActive:   true,
}

func startWebDashboard(port int, onAction func(cmd string, arg string)) {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Type", "text/html; charset=utf-8")
		w.Write([]byte(dashboardHTML))
	})

	http.HandleFunc("/api/state", func(w http.ResponseWriter, r *http.Request) {
		globalWebState.mu.RLock()
		defer globalWebState.mu.RUnlock()
		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(globalWebState)
	})

	http.HandleFunc("/api/action", func(w http.ResponseWriter, r *http.Request) {
		cmd := r.URL.Query().Get("cmd")
		arg := r.URL.Query().Get("arg")
		if cmd != "" && onAction != nil {
			onAction(cmd, arg)
		}
		w.Header().Set("Content-Type", "application/json")
		w.Write([]byte(`{"status":"ok"}`))
	})

	go func() {
		addr := fmt.Sprintf("0.0.0.0:%d", port)
		fmt.Printf("FractureOS Hub Dashboard listening on http://%s\n", addr)
		http.ListenAndServe(addr, nil)
	}()
}

const dashboardHTML = `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>FractureOS Hub Dashboard</title>
<style>
  :root {
    --bg: #0A0E17;
    --panel: #161B22;
    --border: #30363D;
    --cyan: #38BDF8;
    --emerald: #10B981;
    --amber: #F59E0B;
    --rose: #F43F5E;
    --text: #F0F6FC;
    --text-muted: #8B949E;
  }
  body {
    background: var(--bg);
    color: var(--text);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, monospace;
    margin: 0; padding: 24px;
    display: flex; flex-direction: column; align-items: center;
  }
  .container { max-width: 860px; width: 100%; }
  header {
    display: flex; justify-content: space-between; align-items: center;
    border-bottom: 1px solid var(--border); padding-bottom: 16px; margin-bottom: 24px;
  }
  h1 { margin: 0; font-size: 24px; letter-spacing: 1px; color: var(--cyan); }
  .badge { background: #21262D; border: 1px solid var(--border); border-radius: 12px; padding: 4px 12px; font-size: 12px; }
  .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); gap: 16px; margin-bottom: 24px; }
  .card {
    background: var(--panel); border: 1px solid var(--border); border-radius: 8px; padding: 16px;
  }
  .card h3 { margin-top: 0; font-size: 14px; color: var(--text-muted); }
  .btn-group { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 12px; }
  button {
    background: #21262D; color: var(--text); border: 1px solid var(--border);
    border-radius: 6px; padding: 8px 14px; font-weight: 600; cursor: pointer; transition: 0.15s;
  }
  button:hover { background: #30363D; border-color: var(--cyan); }
  button.active { background: var(--cyan); color: #000; border-color: var(--cyan); }
  input[type=range] { width: 100%; accent-color: var(--cyan); margin: 12px 0; }
  input[type=text] {
    width: calc(100% - 20px); background: #0D1117; color: #FFF; border: 1px solid var(--border);
    padding: 8px 10px; border-radius: 6px; margin-bottom: 8px;
  }
</style>
</head>
<body>
<div class="container">
  <header>
    <div>
      <h1>FRACTURE OS HUB</h1>
      <small style="color:var(--text-muted)">TomTom ONE v6 Smart Desk Companion</small>
    </div>
    <div class="badge" id="connStatus">● Target: 192.168.101.115</div>
  </header>

  <div class="grid">
    <div class="card">
      <h3>ACTIVE VIEW SWITCHER</h3>
      <div class="btn-group">
        <button onclick="sendAction('view', '0')">Clock</button>
        <button onclick="sendAction('view', '1')">App Menu</button>
        <button onclick="sendAction('view', '2')">Telemetry</button>
        <button onclick="sendAction('view', '3')">Spotify</button>
        <button onclick="sendAction('view', '4')">Pomodoro</button>
        <button onclick="sendAction('view', '5')">Server</button>
        <button onclick="sendAction('view', '6')">Settings</button>
      </div>
    </div>

    <div class="card">
      <h3>DISPLAY & LUX</h3>
      <label>Brightness: <span id="brightVal">80</span>%</label>
      <input type="range" min="10" max="100" value="80" onchange="sendAction('brightness', this.value)">
      <div class="btn-group">
        <button onclick="sendAction('night_mode', 'toggle')">Toggle Night Mode</button>
        <button onclick="sendAction('sleep', 'now')">Screen Standby</button>
      </div>
    </div>

    <div class="card">
      <h3>DESK QUICK ACTIONS</h3>
      <div class="btn-group">
        <button onclick="sendAction('media', 'play_pause')">⏯ Play/Pause</button>
        <button onclick="sendAction('media', 'next')">⏭ Next Track</button>
        <button onclick="sendAction('audio', 'toggle_mute')">Toggle Mute</button>
        <button onclick="sendAction('pc_active', 'toggle')">Toggle Busy/Zen</button>
      </div>
    </div>

    <div class="card">
      <h3>PUSH TOAST NOTIFICATION</h3>
      <input type="text" id="notifInput" placeholder="Meeting in 5 mins / Discord ping...">
      <button onclick="sendAction('notify', document.getElementById('notifInput').value)">Send to TomTom</button>
    </div>
  </div>
</div>

<script>
function sendAction(cmd, arg) {
  fetch('/api/action?cmd=' + encodeURIComponent(cmd) + '&arg=' + encodeURIComponent(arg))
    .then(r => r.json())
    .then(() => refresh());
}
function refresh() {
  fetch('/api/state')
    .then(r => r.json())
    .then(data => {
      document.getElementById('brightVal').innerText = data.Brightness;
    });
}
setInterval(refresh, 3000);
</script>
</body>
</html>`
