#include <Arduino.h>  //not needed in the arduino ide
#include <WiFi.h>
// Captive Portal
#include <AsyncTCP.h>  
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>	
#include <esp_wifi.h>			//Used for mpdu_rx_disable android workaround
// #include "history.h"  // Include history functions for logging

const char *APssid = "ALC_Master";  // FYI The SSID can't have a space in it.
// const char * password = "12345678"; //Atleast 8 chars
const char *APpassword = NULL;  // no password

#define MAX_CLIENTS 4	
#define WIFI_CHANNEL 6	// 2.4ghz channel 6 https://en.wikipedia.org/wiki/List_of_WLAN_channels#2.4_GHz_(802.11b/g/n/ax)

const IPAddress localIP(4, 3, 2, 1);		   // the IP address the web server, Samsung requires the IP to be in public space
const IPAddress gatewayIP(4, 3, 2, 1);		  
const IPAddress subnetMask(255, 255, 255, 0);  // no need to change: https://avinetworks.com/glossary/subnet-mask/

const String localIPURL = "http://4.3.2.1";	 

const char index_TCP_html[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>
<head>
  <title>HRS232 TCP Config</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    body {
      background: #f4f6fb;
      font-family: 'Segoe UI', Arial, sans-serif;
      margin: 0; padding: 0;
    }
    .container {
      max-width: 420px;
      margin: 40px auto;
      background: #fff;
      border-radius: 12px;
      box-shadow: 0 2px 12px rgba(0,0,0,0.08);
      padding: 32px 28px 24px 28px;
    }
    h1 {
      text-align: center;
      color: #2c3e50;
      margin-bottom: 12px;
    }
    p {
      text-align: center;
      color: #555;
      margin-bottom: 28px;
    }
    label {
      display: block;
      margin-top: 16px;
      margin-bottom: 6px;
      color: #34495e;
      font-weight: 500;
    }
    input[type="text"], input[type="number"], select {
      width: 100%;
      padding: 8px 10px;
      border: 1px solid #d0d7de;
      border-radius: 5px;
      font-size: 1em;
      background: #f8fafc;
      margin-bottom: 2px;
      box-sizing: border-box;
    }
    button {
      margin-top: 22px;
      width: 100%;
      padding: 10px 0;
      background: #0078d7;
      color: #fff;
      border: none;
      border-radius: 6px;
      font-size: 1.1em;
      font-weight: 600;
      cursor: pointer;
      transition: background 0.2s;
    }
    button:hover {
      background: #005fa3;
    }
    .info {
      margin-top: 18px;
      padding: 10px;
      background: #eaf6ff;
      border-left: 4px solid #0078d7;
      color: #333;
      font-size: 0.97em;
      border-radius: 4px;
    }
  </style>
  <script>
    // Lấy config từ server và cập nhật form
    document.addEventListener('DOMContentLoaded', function() {
      fetch('/loadTCPconfig')
        .then(r => r.json())
        .then(cfg => {
          document.querySelector('input[name="tcp_ip"]').value = cfg.tcp_ip || '';
          document.querySelector('input[name="tcp_port"]').value = cfg.tcp_port || '';
          document.querySelector('select[name="tcp_role"]').value = cfg.tcp_role || 'client';
          document.querySelector('input[name="my_ip"]').value = cfg.my_ip || '';
          document.querySelector('input[name="my_gw"]').value = cfg.my_gw || '';
          document.querySelector('input[name="my_sn"]').value = cfg.my_sn || '';
          document.querySelector('input[name="my_dns"]').value = cfg.my_dns || '';
          document.querySelector('input[name="baudrate"]').value = cfg.baudrate || 115200;
          // Hiển thị thông tin hiện tại
          document.getElementById('currentInfo').innerHTML =
            '<b>TCP Role:</b> ' + (cfg.tcp_role || '') + '<br>' +
            '<b>TCP Server:</b> ' + (cfg.tcp_ip || '') + ':' + (cfg.tcp_port || '') + '<br>' +
            '<b>My IP:</b> ' + (cfg.my_ip || '') + '<br>' +
            '<b>Gateway:</b> ' + (cfg.my_gw || '') + '<br>' +
            '<b>Subnet:</b> ' + (cfg.my_sn || '') + '<br>' +
            '<b>DNS:</b> ' + (cfg.my_dns || '') + '<br>' +
            '<b>Baudrate:</b> ' + (cfg.baudrate || 115200);
        });
    });
  </script>
</head>
<body>
  <div class="container">
    <h1>HRS232 TCP Config</h1>
    <p>Cấu hình TCP/IP cho HRS232</p>
    <form action="/set_tcp_ip" method="POST">
      <label>TCP Server IP:</label>
      <input type="text" name="tcp_ip" required>
      <label>TCP Server Port:</label>
      <input type="number" name="tcp_port" min="1" max="65535" required>
      <label>TCP Role:</label>
      <select name="tcp_role">
        <option value="server">Server</option>
        <option value="client">Client</option>
      </select>
      <label>My IP:</label>
      <input type="text" name="my_ip" required>
      <label>My Gateway:</label>
      <input type="text" name="my_gw" required>
      <label>My Subnet:</label>
      <input type="text" name="my_sn" required>
      <label>My DNS:</label>
      <input type="text" name="my_dns" required>
      <label>Client Baudrate:</label>
      <input type="number" name="baudrate" value="115200" min="300" max="1000000" required>
      <button type="submit">Lưu cấu hình</button>
    </form>
    <div class="info" id="currentInfo">
      Đang tải cấu hình...
    </div>
    <button type="button" onclick="fetch('/api/reset').then(r=>r.text()).then(alert)">Reset ESP (API)</button>
    <button type="button" onclick="fetch('/console').then(r=>r.text()).then(alert)">Xem Console</button>
  </div>
</body>
</html>
)=====";

const char index_html[] PROGMEM = R"====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>HRS232 Dashboard</title>
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <style>
    body {
      background: #f4f6fb;
      font-family: 'Segoe UI', Arial, sans-serif;
      margin: 0; padding: 0;
    }
    .dashboard {
      max-width: 1100px;
      margin: 40px auto;
      display: flex;
      flex-wrap: wrap;
      gap: 28px;
      justify-content: center;
    }
    .card {
      background: #fff;
      border-radius: 14px;
      box-shadow: 0 2px 12px rgba(0,0,0,0.08);
      padding: 28px 24px 20px 24px;
      width: 240px;
      position: relative;
      display: flex;
      flex-direction: column;
      align-items: stretch;
    }
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 18px;
    }
    .card-id {
      font-weight: bold;
      color: #0078d7;
      font-size: 1.1em;
    }
    .card-name {
      font-size: 1.1em;
      color: #2c3e50;
      margin-left: 8px;
      flex: 1;
    }
    .setting-btn {
      background: #eaf6ff;
      border: none;
      border-radius: 5px;
      color: #0078d7;
      font-size: 1.1em;
      padding: 4px 10px;
      cursor: pointer;
      transition: background 0.2s;
    }
    .setting-btn:hover {
      background: #cbe6ff;
    }
    .input-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 12px;
    }
    .input-row label {
      color: #34495e;
      font-weight: 500;
      width: 60px;
    }
    .input-row input {
      width: 110px;
      padding: 6px 8px;
      border: 1px solid #d0d7de;
      border-radius: 5px;
      font-size: 1em;
      background: #f8fafc;
      text-align: right;
    }
    .reset-btn {
      margin-top: 10px;
      background: #ff4d4f;
      color: #fff;
      border: none;
      border-radius: 6px;
      font-size: 1em;
      font-weight: 600;
      padding: 7px 0;
      cursor: pointer;
      transition: background 0.2s;
    }
    .reset-btn:hover {
      background: #d9363e;
    }
    /* Modal styles */
    .modal {
      display: none;
      position: fixed;
      z-index: 1000;
      left: 0; top: 0; width: 100vw; height: 100vh;
      background: rgba(0,0,0,0.25);
      justify-content: center;
      align-items: center;
    }
    .modal-content {
      background: #fff;
      border-radius: 10px;
      padding: 28px 32px 18px 32px;
      min-width: 320px;
      box-shadow: 0 2px 16px rgba(0,0,0,0.15);
      position: relative;
    }
    .modal-header {
      font-size: 1.2em;
      font-weight: bold;
      color: #0078d7;
      margin-bottom: 16px;
    }
    .modal-row {
      margin-bottom: 14px;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .modal-row label {
      width: 90px;
      color: #34495e;
      font-weight: 500;
    }
    .modal-row input {
      flex: 1;
      padding: 6px 8px;
      border: 1px solid #d0d7de;
      border-radius: 5px;
      font-size: 1em;
      background: #f8fafc;
    }
    .modal-actions {
      display: flex;
      justify-content: flex-end;
      gap: 10px;
      margin-top: 10px;
    }
    .modal-btn {
      background: #0078d7;
      color: #fff;
      border: none;
      border-radius: 6px;
      font-size: 1em;
      font-weight: 600;
      padding: 7px 18px;
      cursor: pointer;
      transition: background 0.2s;
    }
    .modal-btn.cancel {
      background: #aaa;
    }
    .modal-btn:hover:not(.cancel) {
      background: #005fa3;
    }
    .modal-btn.cancel:hover {
      background: #888;
    }
    .modal-error {
      color: #d9363e;
      font-size: 0.98em;
      margin-bottom: 8px;
      min-height: 18px;
    }
  </style>
</head>
<body>
  <div style="text-align:center; margin-top:24px;">
    <h1>ALC Dashboard</h1>
    <p>Giám sát và cấu hình 4 Counter</p>
  </div>
  <div class="dashboard" id="dashboard">
    <!-- Cards will be rendered here -->
  </div>

  <!-- Modal dùng chung -->
  <div class="modal" id="settingModal">
    <div class="modal-content">
      <div class="modal-header" id="modalTitle">Cài đặt Counter</div>
      <div class="modal-error" id="modalError"></div>
      <form id="modalForm" autocomplete="off">
        <div id="modalSettingFields" style="display:block;">
          <div class="modal-row">
            <label>Tên:</label>
            <input type="text" id="modalName" name="modalName"  required>
          </div>
          <div class="modal-row">
            <label>Bội số:</label>
            <input type="number" id="modalInc" min="1" max="1000" required>
          </div>
          <div class="modal-row">
            <label>Filter:</label>
            <input type="number" id="modalFilter" min="0" max="10000" required>
          </div>
          <div class="modal-row">
            <label>Địa chỉ:</label>
            <input type="text" id="modalAddress" placeholder="0-65535" required>
          </div>
          <div class="modal-row">
            <label>Pin chức năng:</label>
            <input type="number" id="modalFuncPin" min="0" max="48" required>
          </div>
          <div class="modal-row">
            <label>Đổi mật khẩu:</label>
            <input type="password" id="modalNewPassword" placeholder="Nhập mật khẩu mới (nếu cần)">
        </div>
        <div class="modal-actions">
          <button type="button" class="modal-btn cancel" onclick="closeModal()">Hủy</button>
          <button type="submit" class="modal-btn" id="modalSubmitBtn">OK</button>
          <button class="modal-btn" style="position:absolute; top:10px; right:10px;" onclick="window.location.href='/lora'">Wireless Setting</button>
        </div>
      </form>
    </div>
  </div>

  <script>
    // --- Dashboard Data ---
    let counters = [
      {id: 1, name: "Counter 1", plan: "", result: "", inc: 1, filter: 0},
      {id: 2, name: "Counter 2", plan: "", result: "", inc: 1, filter: 0},
      {id: 3, name: "Counter 3", plan: "", result: "", inc: 1, filter: 0},
      {id: 4, name: "Counter 4", plan: "", result: "", inc: 1, filter: 0}
    ];

    // --- Render Dashboard ---
    function renderDashboard() {
      const dash = document.getElementById('dashboard');
      dash.innerHTML = '';
      counters.forEach((c, idx) => {
        dash.innerHTML += `
        <div class="card" data-id="${c.id}">
          <div class="card-header">
            <span class="card-id">#${c.id}</span>
            <span class="card-name" id="name-${c.id}">${c.name}</span>
            <button class="setting-btn" onclick="openSettingModal(${c.id})">&#9881;</button>
          </div>
          <div class="input-row">
            <label>Plan</label>
            <input type="number" id="plan-${c.id}" value="${c.plan}" onchange="updatePlan(${c.id}, this.value)">
          </div>
          <div class="input-row">
            <label>Result</label>
            <input type="number" id="result-${c.id}" value="${c.result}" onchange="updateResult(${c.id}, this.value)">
          </div>
          <button class="reset-btn" onclick="resetCounter(${c.id})">Reset</button>
        </div>
        `;
      });
    }
    
    function updatePlan(id, value) {
        const params = new URLSearchParams();
        params.append('id', id);
        params.append('plan', value);
        fetch('/api/plan', {
            method: 'POST',
            headers: {'Content-Type':'application/x-www-form-urlencoded'},
            body: params.toString()
        }).then(() => {
            counters[id - 1].plan = value;
        });
    }

    function updateResult(id, value) {
        const params = new URLSearchParams();
        params.append('id', id);
        params.append('result', value);
        fetch('/api/result', {
            method: 'POST',
            headers: {'Content-Type':'application/x-www-form-urlencoded'},
            body: params.toString()
        }).then(() => {
            counters[id - 1].result = value;
        });
    }

    // --- Fetch Plan/Result from /api/param ---
    function fetchParams() {
      fetch('/api/param')
      .then(res => res.json())
      .then(data => {
        let totalPlan = 0, totalResult = 0;
        data.forEach((c, idx) => {
        counters[idx].plan = c.plan;
        counters[idx].result = c.result;
        counters[idx].name = c.name || counters[idx].name;
        document.getElementById('plan-' + (idx + 1)).value = c.plan;
        document.getElementById('result-' + (idx + 1)).value = c.result;
        document.getElementById('name-' + (idx + 1)).textContent = c.name || counters[idx].name;
        totalPlan += Number(c.plan) || 0;
        totalResult += Number(c.result) || 0;
        });
        updateTotalCard(totalPlan, totalResult);
      });
    }

    // --- Add Total Card ---
    function updateTotalCard(plan, result) {
      let totalCard = document.getElementById('total-card');
      if (!totalCard) {
      const dash = document.getElementById('dashboard');
      totalCard = document.createElement('div');
      totalCard.className = 'card';
      totalCard.id = 'total-card';
      totalCard.style.width = '520px';
      totalCard.style.textAlign = 'center';
      totalCard.style.fontSize = '2.2em';
      totalCard.style.background = '#eaf6ff';
      totalCard.style.marginBottom = '18px';
      dash.parentNode.insertBefore(totalCard, dash);
      }
      totalCard.innerHTML = `
      <div style="font-size:1.2em;color:#0078d7;font-weight:bold;margin-bottom:10px;">TỔNG KẾT</div>
      <div style="display:flex;justify-content:space-around;">
        <div>
        <div style="color:#34495e;font-size:0.7em;">Total Plan</div>
        <div style="font-size:2.5em;font-weight:bold;color:#0078d7;">${plan}</div>
        </div>
        <div>
        <div style="color:#34495e;font-size:0.7em;">Total Result</div>
        <div style="font-size:2.5em;font-weight:bold;color:#d9363e;">${result}</div>
        </div>
      </div>
      `;
    }

    // --- Reset Counter ---
    function resetCounter(id) {
      if (!confirm('Reset Result về 0 cho Counter #' + id + '?')) return;
      // Gửi JSON: {"cmnd":"resetCounter","id":4}
      fetch('/api/reset', {
        method: 'POST',
        headers: {'Content-Type':'application/json'},
        body: JSON.stringify({cmnd: "resetCounter", id: id})
      }).then(() => {
        document.getElementById('result-' + id).value = 0;
      });
    }

    // --- Modal Logic ---
    let modalCounterId = null;
    function openSettingModal(id) {
      fetchCounterDataToModal(id);

      modalCounterId = id;
      document.getElementById('settingModal').style.display = 'flex';
      document.getElementById('modalTitle').textContent = 'Cài đặt Counter #' + id;
      // document.getElementById('modalPasswordRow').style.display = '';
      document.getElementById('modalSettingFields').style.display = 'block';
      document.getElementById('modalPassword').value = '';
      document.getElementById('modalError').textContent = '';
      document.getElementById('modalName').value = counters[id-1].name;
      document.getElementById('modalInc').value = counters[id-1].inc;
      document.getElementById('modalFuncPin').value = counters[id-1].pin || 0; // Assuming pin is stored in counters
      document.getElementById('modalFilter').value = counters[id-1].filter;
      document.getElementById('modalAddress').value = counters[id-1].id; // Assuming id is the address
      document.getElementById('modalNewPassword').value = ''; // Clear new password field
      document.getElementById('modalPassword').focus();
    }
    function closeModal() {
      document.getElementById('settingModal').style.display = 'none';
    }

    // --- Modal Submit ---
    document.getElementById('modalForm').onsubmit = function(e) {
    e.preventDefault();
    // Bỏ qua kiểm tra mật khẩu
    const errorDiv = document.getElementById('modalError');
    const id = modalCounterId;
    const password = document.getElementById('modalNewPassword').value || '';
    if (password && password.length < 6) {
        errorDiv.textContent = 'Mật khẩu mới phải ít nhất 6 ký tự!';
        return;
    }
    const name = document.getElementById('modalName').value;
    const inc = parseInt(document.getElementById('modalInc').value);
    const filter = parseInt(document.getElementById('modalFilter').value);
    const pin = parseInt(document.getElementById('modalFuncPin').value) || 0;
    const address = document.getElementById('modalAddress').value;
    const params = new URLSearchParams();
    params.append('id', id);
    params.append('name', name);
    params.append('filter', filter);
    params.append('inc', inc);
    params.append('pin', pin);
    params.append('password', password);
    params.append('address', address);

    fetch('/api/setting', {
        method: 'POST',
        headers: {'Content-Type':'application/x-www-form-urlencoded'},
        body: params.toString()
    }).then(() => {
        counters[id-1].name = name;
        counters[id-1].filter = filter;
        counters[id-1].inc = inc;
        counters[id-1].pin = pin;
        if(password !== '') {
        counters[id-1].password = password;
        }
        counters[id-1].id = address;
        closeModal();
    });
    };
    // Hàm fetch để cập nhật data lên modal
    function fetchCounterDataToModal(id) {
      fetch('/api/counter?id=' + id)
        .then(res => res.json())
        .then(counter => {
          updateModalFields(counter);
        });
    }

    function updateModalFields(counter) {
      document.getElementById('modalName').value = counter.name || '';
      document.getElementById('modalInc').value = counter.inc || 1;
      document.getElementById('modalFilter').value = counter.filter || 0;
      document.getElementById('modalFuncPin').value = counter.pin || 0;
      document.getElementById('modalAddress').value = counter.id || '';
      document.getElementById('modalNewPassword').value = '';
    }

    // --- WebSocket cập nhật realtime ---
    const socket = new WebSocket('ws://' + location.host + '/ws');
    socket.onmessage = function(event) {
      // Cập nhật khi nhận chuỗi dạng {"id":2,"result":45,"plan":0,"name":""}
      try {
        const data = JSON.parse(event.data);
        if (typeof data.id !== "undefined") {
          const idx = data.id - 1;
          if (counters[idx]) {
            if (typeof data.plan !== "undefined") {
              counters[idx].plan = data.plan;
              const planInput = document.getElementById('plan-' + data.id);
              if (planInput) planInput.value = data.plan;
            }
            if (typeof data.result !== "undefined") {
              counters[idx].result = data.result;
              const resultInput = document.getElementById('result-' + data.id);
              if (resultInput) resultInput.value = data.result;
            }
            if (typeof data.name !== "undefined") {
              counters[idx].name = data.name || ("Counter " + data.id);
              const nameSpan = document.getElementById('name-' + data.id);
              if (nameSpan) nameSpan.textContent = counters[idx].name;
            }
          }
        }
      } catch (e) {
        // fallback for other message types if needed
      }
    };

    // --- Khởi tạo ---
    renderDashboard();
  </script>
</body>
</html>
)====";
// Trang HTML console
const char index_console[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>HRS232 Console</title>
  <meta charset="utf-8">
  <style>
    body { background: #222; color: #eee; font-family: monospace; }
    #console { width: 100%; height: 90vh; background: #111; color: #0f0; padding: 10px; overflow-y: scroll; border: 1px solid #444; }
  </style>
</head>
<body>
  <h2>HRS232 Console</h2>
  <div id="console"></div>
  <input type="text" id="input" placeholder="Type here..." autofocus
         onkeydown="if(event.key === 'Enter') { ws.send(this.value); this.value=''; }">
  <script>
    var ws = new WebSocket('ws://' + location.host + '/ws_console');
    var consoleDiv = document.getElementById('console');
    ws.onmessage = function(event) {
      consoleDiv.innerText += event.data;
      consoleDiv.scrollTop = consoleDiv.scrollHeight;
    };
    ws.onopen = function() { consoleDiv.textContent += '[Console connected]\n'; };
    ws.onclose = function() { consoleDiv.textContent += '[Console disconnected]\n'; };
  </script>
</body>
</html>
)rawliteral";

bool WARNING = false; // Biến để theo dõi trạng thái cảnh báo
bool ERROR = false;

// WebSocket cho console
AsyncWebSocket wsConsole("/ws_console");
#include <vector>

bool serverStarted = false;
bool socketConnected = false;
// Lưu trữ các dòng console
std::vector<String> consoleBuffer;
const size_t MAX_CONSOLE_LINES = 200;
#include "esp_heap_caps.h"
ProjectConfig WifigConfig;

void consolePrintln(const String& line);
void consolePrintf(const char* format, ...);

// Hàm kiểm tra và khởi tạo PSRAM
void initPSRAM() {
  if (psramInit()) {
    consolePrintln("PSRAM initialized successfully");
    consolePrintf("Total PSRAM: %u bytes\n", ESP.getPsramSize());
    consolePrintf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
  } else {
    consolePrintln("PSRAM initialization failed");
    WARNING = true;
  }
}

// Gọi hàm này trong setup() nếu cần
// Hàm thêm dòng vào consoleBuffer
void addConsoleLine(const String& line) {
  if (consoleBuffer.size() >= MAX_CONSOLE_LINES) {
    consoleBuffer.erase(consoleBuffer.begin());
  }
  consoleBuffer.push_back(line);
  // Gửi dòng mới đến WebSocket console
  if (socketConnected) {
    wsConsole.textAll(line);
  }
}

 void consolePrintln(const String& line) {
  Serial.println(line);
  addConsoleLine(line + '\n');
}

void consolePrintf(const char* format, ...) {
  char buf[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  Serial.print(buf);
  addConsoleLine(String(buf));
}

// Gửi toàn bộ buffer cho client mới
void sendConsoleBuffer(AsyncWebSocketClient *client) {
  String all;
  for (const auto& l : consoleBuffer) {
    // Lọc bỏ ký tự không phải ASCII hoặc UTF-8 hợp lệ
    String safeLine;
    for (size_t i = 0; i < l.length(); ++i) {
      char c = l[i];
      if ((c >= 32 && c <= 126) || c == '\n' || c == '\r') { // chỉ gửi ký tự in được
        safeLine += c;
      }
    }
    all += safeLine + "\n";
  }
  client->text(all);
}

// Khi có client mới kết nối WebSocket
void onWsConsoleEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    sendConsoleBuffer(client);
    socketConnected = true;
  }
  else if (type == WS_EVT_DISCONNECT) {
    consolePrintln("Client disconnected from console WebSocket");
    socketConnected = false;
  } else if (type == WS_EVT_DATA) {
    // Không cần xử lý dữ liệu từ client, chỉ gửi dữ liệu console
    Serial1.write(data, len); // Ghi dữ liệu nhận được từ client vào Serial1
    consolePrintln("Received from console WebSocket: " + String((char*)data, len));
  }
}


void Console_setup(AsyncWebServer &server) {
  // Đăng ký WebSocket cho console
  wsConsole.onEvent(onWsConsoleEvent);
  server.addHandler(&wsConsole);

  // Trang HTML console
  server.on("/console", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_console);
  });

  // Gửi thông báo khi kết nối thành công
  consolePrintln("Console WebSocket initialized");
}

DNSServer dnsServer;
// AsyncWebServer server(80);

void setUpDNSServer(DNSServer &dnsServer, const IPAddress &localIP) {
// Define the DNS interval in milliseconds between processing DNS requests
#define DNS_INTERVAL 30

	// Set the TTL for DNS response and start the DNS server
	dnsServer.setTTL(3600);
	dnsServer.start(53, "*", localIP);
}

void startSoftAccessPoint(const char *ssid, const char *password, const IPAddress &localIP, const IPAddress &gatewayIP) {
// Define the maximum number of clients that can connect to the server
#define MAX_CLIENTS 4
// Define the WiFi channel to be used (channel 6 in this case)
#define WIFI_CHANNEL 6

	// Set the WiFi mode to access point and station
	WiFi.mode(WIFI_MODE_AP);

	// Define the subnet mask for the WiFi network
	const IPAddress subnetMask(255, 255, 255, 0);

	// Configure the soft access point with a specific IP and subnet mask
	WiFi.softAPConfig(localIP, gatewayIP, subnetMask);

	// Start the soft access point with the given ssid, password, channel, max number of clients
	WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);

	// Disable AMPDU RX on the HRS232 WiFi to fix a bug on Android
	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();
	vTaskDelay(100 / portTICK_PERIOD_MS);  // Add a small delay
}

void setUpWebserver(AsyncWebServer &server, const IPAddress &localIP) {
  Console_setup(server);  // Initialize console WebSocket
	//======================== Webserver ========================
	// WARNING IOS (and maybe macos) WILL NOT POP UP IF IT CONTAINS THE WORD "Success" https://www.esp8266.com/viewtopic.php?f=34&t=4398
	// SAFARI (IOS) IS STUPID, G-ZIPPED FILES CAN'T END IN .GZ https://github.com/homieiot/homie-esp8266/issues/476 this is fixed by the webserver serve static function.
	// SAFARI (IOS) there is a 128KB limit to the size of the HTML. The HTML can reference external resources/images that bring the total over 128KB
	// SAFARI (IOS) popup browser has some severe limitations (javascript disabled, cookies disabled)

	// Required
	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });	// windows 11 captive portal workaround
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) { request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

	// Background responses: Probably not all are Required, but some are. Others might speed things up?
	// A Tier (commonly used by modern systems)
	server.on("/generate_204", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });		   // android captive portal redirect
	server.on("/redirect", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // microsoft redirect
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });  // apple call home
	server.on("/canonical.html", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });	   // firefox captive portal call home
	server.on("/success.txt", [](AsyncWebServerRequest *request) { request->send(200); });					   // firefox captive portal call home
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // windows call home

	// B Tier (uncommon)
	//  server.on("/chrome-variations/seed",[](AsyncWebServerRequest *request){request->send(200);}); //chrome captive portal call home
	//  server.on("/service/update2/json",[](AsyncWebServerRequest *request){request->send(200);}); //firefox?
	//  server.on("/chat",[](AsyncWebServerRequest *request){request->send(404);}); //No stop asking Whatsapp, there is no internet connection
	//  server.on("/startpage",[](AsyncWebServerRequest *request){request->redirect(localIPURL);});

	// return 404 to webpage icon
	server.on("/favicon.ico", [](AsyncWebServerRequest *request) { request->send(404); });	// webpage icon

	// Serve Basic HTML Page
	server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
		AsyncWebServerResponse *response = request->beginResponse(200, "text/html", index_html);
		response->addHeader("Cache-Control", "public");  // save this file to cache for 1 year (unless you refresh)
		request->send(response);
		consolePrintln("Served Basic HTML Page");
	});
  server.on("/history", HTTP_GET, [](AsyncWebServerRequest *request) {
    // handleHistory(request);
  });
	// the catch all
	// server.onNotFound([](AsyncWebServerRequest *request) {
	// 	request->redirect(localIPURL);
	// 	Serial.print("onnotfound ");
	// 	Serial.print(request->host());	// This gives some insight into whatever was being requested on the serial monitor
	// 	Serial.print(" ");
	// 	Serial.print(request->url());
	// 	Serial.print(" sent redirect to " + localIPURL + "\n");
	// });
}

void WiFiOnEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_START:
      consolePrintln("WiFi AP started");
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
      consolePrintln("WiFi AP stopped");
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      consolePrintln("Client connected to AP");
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      consolePrintln("Client disconnected from AP");
      break;
    default:
      consolePrintf("WiFi Event: %d\n", event);
      break;
  }
}

void WiFi_AP_setup(AsyncWebServer &server) {
  char ssidWithMac[32];
  WiFi.softAPdisconnect(true); // Reset WiFi before initializing new AP
	// Print a welcome message to the Serial port.
	consolePrintln("\n\nHRS232, V0.1.0 compiled " __DATE__ " " __TIME__ " by VinhPhat");  //__DATE__ is provided by the platformio ide
	Serial.printf("%s-%d\n\r", ESP.getChipModel(), ESP.getChipRevision());

  Serial.println("Starting WiFi AP...");
  	WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
	// Start the soft access point with the given ssid, password, channel, max number of clients
  String ssid = APssid;
  if (WifigConfig.lineName[0] != '\0') {
      char ssidBuf[64];
      snprintf(ssidBuf, sizeof(ssidBuf), "ALC[%s]", WifigConfig.lineName);
      ssid = ssidBuf;
  }
  WiFi.softAP(ssid.c_str(), APpassword, WIFI_CHANNEL, 0, MAX_CLIENTS);
  delay(200);
  // Khởi tạo WiFi AP trước   
    uint8_t mac[6];
    if (esp_wifi_get_mac(WIFI_IF_AP, mac) == ESP_OK) {
        
        snprintf(ssidWithMac, sizeof(ssidWithMac), "%s_%02X%02X%02X", ssid, mac[3], mac[4], mac[5]);
        // Đổi SSID bằng WiFi.softAP

        startSoftAccessPoint(ssidWithMac, APpassword, localIP, gatewayIP);delay(200);
        Serial.println("  ✅ Updated AP SSID: " + String(ssidWithMac));
    } else {
        Serial.println("Failed to get AP MAC!");
    }
    delay(200);

	setUpDNSServer(dnsServer, localIP);
  // ALC_setup(server); // Khởi tạo ALC Project
  Serial.println("Setting up Webserver...");
	setUpWebserver(server, localIP);
  Serial.println("  ✅ Webserver setup complete.");
	server.begin();
  Serial.println("WiFi AP started with SSID: " + String(ssidWithMac));
	Serial.print("\n");
	Serial.print("Startup Time:");	// should be somewhere between 270-350 for Generic HRS232 (D0WDQ6 chip, can have a higher startup time on first boot)
	Serial.println(millis());
  consolePrintln("Startup Time:" + String(millis()));
	Serial.print("\n");
  // Register WiFi event handler
  WiFi.onEvent(WiFiOnEvent);  // Register the WiFi event handler
}

void WiFi_AP_loop() {
	dnsServer.processNextRequest();	 // I call this atleast every 10ms in my other projects (can be higher but I haven't tested it for stability)
	delay(DNS_INTERVAL);			 // seems to help with stability, if you are doing other things in the loop this may not be needed
}

