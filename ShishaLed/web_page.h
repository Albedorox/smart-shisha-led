// Web-App (eine Seite, ohne externe Dateien), ausgeliefert unter "/".
// Fragt /api/state alle 500 ms ab; Einstellungen per POST an /api/config, /api/name, /api/reset.
#pragma once

#include <pgmspace.h>

static const char WEB_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="de"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Shisha LED</title>
<style>
body{font-family:system-ui,sans-serif;margin:0;padding:16px;background:#111;color:#eee;max-width:520px;margin:auto}
h1{font-size:1.3em;margin:0 0 12px}h2{font-size:1em;margin:20px 0 8px;color:#aaa}
.card{background:#1c1c1c;border-radius:12px;padding:14px;margin-bottom:12px}
#dot{width:64px;height:64px;border-radius:50%;margin:0 auto 8px;background:#222;box-shadow:0 0 24px #000}
#timer{font-size:2.4em;text-align:center;font-variant-numeric:tabular-nums}
.row{display:flex;gap:8px;align-items:center;margin:6px 0}.row label{flex:1}
input,select,button{font:inherit;padding:8px;border-radius:8px;border:1px solid #333;background:#222;color:#eee}
input[type=number]{width:90px}button{background:#2a6;border:0;cursor:pointer}button.sec{background:#444}
ol{padding-left:22px;margin:0}li{margin:4px 0}.muted{color:#888;font-size:.85em}#err{color:#f66}
</style></head><body>
<h1>Shisha LED</h1>
<div class="card"><div id="dot"></div><div id="timer">0,0 s</div>
<div class="muted" style="text-align:center">Letzter Zug: <span id="last">–</span> · Züge: <span id="count">0</span></div></div>
<div class="card"><div class="row"><input id="name" maxlength="24" placeholder="Dein Name" style="flex:1"><button onclick="setName()">Übernehmen</button></div>
<div class="muted">Aktuell: <span id="cur"></span> (gilt ab dem nächsten Zug)</div></div>
<div class="card"><h2 style="margin-top:0">Top 5</h2><ol id="lb"></ol>
<div class="row" style="margin-top:10px"><button class="sec" onclick="reset('leaderboard')">Rangliste leeren</button></div></div>
<div class="card"><h2 style="margin-top:0">Licht</h2>
<div class="row"><label>Modus</label><select id="mode"><option value="smart">Smart (Zuglicht)</option><option value="steady">Dauerlicht</option><option value="off">Aus</option></select></div>
<div class="row"><label>Helligkeit</label><input id="brightness" type="range" min="0" max="255"></div>
<div class="row"><label>Dauerlicht-Farbe</label><input id="color" type="color"></div>
<div class="row"><label>Bereitschaftsglimmen</label><input id="idleGlow" type="checkbox"></div>
<h2>Verlauf und Sensor</h2>
<div class="row"><label>Gelb nach (ms)</label><input id="yellowMs" type="number" step="100"></div>
<div class="row"><label>Rot nach (ms)</label><input id="redMs" type="number" step="100"></div>
<div class="row"><label>Nachlauf Zugende (ms)</label><input id="tailMs" type="number" step="50"></div>
<div class="row"><label>Diagnosemodus (zählt nicht)</label><input id="diagnostic" type="checkbox"></div>
<div class="row"><button onclick="save()">Speichern</button><button class="sec" onclick="reset('all')">Standardwerte</button></div>
<div id="err"></div>
<div class="muted" id="sensor"></div></div>
<div class="muted">Firmware <span id="fw"></span></div>
<script>
const $=id=>document.getElementById(id);let editing=false,loaded=false;
document.querySelectorAll('input,select').forEach(e=>{e.addEventListener('focus',()=>editing=true);e.addEventListener('blur',()=>editing=false)});
const sec=ms=>(ms/1000).toFixed(1).replace('.',',')+' s';
const hex=c=>'#'+c.map(v=>v.toString(16).padStart(2,'0')).join('');
function esc(s){const d=document.createElement('div');d.textContent=s;return d.innerHTML}
async function post(url,data){const r=await fetch(url,{method:'POST',body:new URLSearchParams(data)});const j=await r.json();
 if(!r.ok){$('err').textContent='Ungültiger Wert: '+j.error;return}$('err').textContent='';render(j,true)}
function render(s,force){
 $('fw').textContent=s.fw;$('cur').textContent=s.name;$('count').textContent=s.pullCount;
 $('timer').textContent=sec(s.pull.elapsedMs);
 $('dot').style.background=s.config.mode=='off'?'#222':(s.config.mode=='steady'?hex(s.config.steadyColor):hex(s.color));
 $('last').textContent=s.lastPull?sec(s.lastPull.durationMs)+' ('+s.lastPull.name+')':'–';
 $('lb').innerHTML=s.leaderboard.map(e=>'<li>'+esc(e.name)+' – '+sec(e.durationMs)+(e.truncated?' (max)':'')+'</li>').join('')||'<span class="muted">noch keine Züge</span>';
 $('sensor').textContent='Sensor: '+(s.sensor.level?'aktiv':'ruhig')+', '+s.sensor.pulses10s+' Impulse/10 s'+(s.sensor.fault?' – Dauersignal! Poti zurückdrehen':'');
 if((!editing&&!loaded)||force){const c=s.config;$('mode').value=c.mode;$('brightness').value=c.brightness;$('color').value=hex(c.steadyColor);
  $('idleGlow').checked=c.idleGlow;$('yellowMs').value=c.yellowMs;$('redMs').value=c.redMs;$('tailMs').value=c.tailMs;$('diagnostic').checked=c.diagnostic;loaded=true}
}
function save(){const c=$('color').value;post('/api/config',{mode:$('mode').value,brightness:$('brightness').value,
 r:parseInt(c.substr(1,2),16),g:parseInt(c.substr(3,2),16),b:parseInt(c.substr(5,2),16),
 yellowMs:$('yellowMs').value,redMs:$('redMs').value,tailMs:$('tailMs').value,
 idleGlow:$('idleGlow').checked?1:0,diagnostic:$('diagnostic').checked?1:0})}
function setName(){post('/api/name',{name:$('name').value})}
function reset(scope){if(confirm(scope=='leaderboard'?'Rangliste leeren?':'Alle Einstellungen zurücksetzen?'))post('/api/reset',{scope})}
async function poll(){try{const r=await fetch('/api/state');render(await r.json(),false)}catch(e){}setTimeout(poll,500)}
poll();
</script></body></html>)HTML";
