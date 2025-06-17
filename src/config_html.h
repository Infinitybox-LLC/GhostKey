#ifndef CONFIG_HTML_H
#define CONFIG_HTML_H

const char config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Ghost Key Configuration</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            text-align: center; 
            margin: 0px auto; 
            padding: 20px; 
            background-color: #f5f5f5;
        }
        .logo { 
            position: absolute; 
            top: 20px; 
            left: 20px; 
            width: 100px; 
            height: auto; 
        }
        .container { 
            margin-top: 60px; 
            max-width: 800px;
            margin-left: auto;
            margin-right: auto;
            background-color: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        .button { 
            background-color: rgb(170, 0, 225); 
            border: none; 
            color: white; 
            padding: 16px 40px;
            text-decoration: none; 
            font-size: 18px; 
            margin: 4px; 
            cursor: pointer; 
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .button:hover {
            background-color: rgb(140, 0, 195);
        }
        .button2 { 
            background-color: #555555; 
        }
        .button2:hover {
            background-color: #333333;
        }
        .form-group { 
            margin: 20px 0; 
            text-align: left;
        }
        .form-group label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
        }
        input[type=number] { 
            padding: 8px; 
            font-size: 16px; 
            width: 150px; 
            border: 1px solid #ddd;
            border-radius: 3px;
        }
        .popup { 
            position: fixed; 
            top: 20px; 
            left: 50%; 
            transform: translateX(-50%); 
            background-color: #4CAF50; 
            color: white; 
            padding: 15px 30px; 
            border-radius: 5px; 
            display: none; 
            z-index: 1000; 
        }
        .device { 
            border: 1px solid #ccc; 
            padding: 15px; 
            margin: 10px 0; 
            border-radius: 5px;
            background-color: #fafafa;
        }
        .priority { 
            background-color: #e6ffe6; 
            border-color: #4CAF50;
        }
        .signal { 
            font-weight: bold; 
        }
        .good { 
            color: green; 
        }
        .medium { 
            color: orange; 
        }
        .poor { 
            color: red; 
        }
        .status { 
            background-color: #f0f0f0; 
            padding: 15px; 
            border-radius: 5px; 
            margin: 15px 0; 
            text-align: left;
        }
        .name-input { 
            padding: 8px; 
            margin: 5px; 
            border: 1px solid #ccc; 
            border-radius: 3px; 
            width: 200px;
        }
        .remove { 
            background-color: #f44336; 
        }
        .remove:hover {
            background-color: #d32f2f;
        }
        .tabs {
            margin-bottom: 20px;
        }
        .tab-content {
            text-align: left;
        }
        h1 {
            color: rgb(170, 0, 225);
            margin-bottom: 30px;
        }
        h2 {
            color: #333;
            border-bottom: 2px solid rgb(170, 0, 225);
            padding-bottom: 5px;
        }
        .status-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin: 15px 0;
        }
        .status-item {
            background-color: #f8f9fa;
            padding: 10px;
            border-radius: 5px;
            border-left: 4px solid rgb(170, 0, 225);
        }
        .device-actions {
            margin-top: 10px;
        }
        .device-actions button {
            margin-right: 10px;
            margin-bottom: 5px;
        }
    </style>
    <script>
        // Utility Functions
        function showPopup(message) {
            var popup = document.getElementById('popup');
            popup.textContent = message;
            popup.style.display = 'block';
            setTimeout(function() { 
                popup.style.display = 'none'; 
            }, 5000);
        }

        function showTab(tabName) {
            var mainTab = document.getElementById('mainTabContent');
            var bluetoothTab = document.getElementById('bluetoothTabContent');
            var mainButton = document.getElementById('mainTab');
            var bluetoothButton = document.getElementById('bluetoothTab');
            
            if (tabName === 'main') {
                mainTab.style.display = 'block';
                bluetoothTab.style.display = 'none';
                mainButton.className = 'button';
                bluetoothButton.className = 'button button2';
                loadSystemStatus();
            } else if (tabName === 'bluetooth') {
                mainTab.style.display = 'none';
                bluetoothTab.style.display = 'block';
                mainButton.className = 'button button2';
                bluetoothButton.className = 'button';
                updateDevices();
            }
        }

        // System Status Functions
        async function loadSystemStatus() {
            try {
                const response = await fetch('/status');
                if (response.ok) {
                    const data = await response.json();
                    document.getElementById('systemState').textContent = data.state || 'Unknown';
                    document.getElementById('rfidStatus').textContent = data.rfid ? 'Authenticated' : 'Not Authenticated';
                    document.getElementById('bluetoothStatus').textContent = data.bluetooth ? 'Authenticated' : 'Not Authenticated';
                    document.getElementById('storedKeys').textContent = data.keys || '0';
                    
                    // Update form values
                    if (data.starterPulse) {
                        document.getElementById('pulse_time').value = data.starterPulse;
                    }
                    if (data.autoLockTimeout) {
                        document.getElementById('auto_lock').value = data.autoLockTimeout;
                    }
                }
            } catch (error) {
                console.error('Error loading system status:', error);
            }
        }

        // Configuration Functions
        async function updatePulseTime(event) {
            event.preventDefault();
            var form = event.target;
            var formData = new FormData(form);
            
            try {
                const response = await fetch('/update_pulse', {
                    method: 'POST',
                    body: formData
                });
                const data = await response.text();
                showPopup(data);
            } catch (error) {
                showPopup('Error: ' + error.message);
            }
        }

        async function updateAutoLock(event) {
            event.preventDefault();
            var form = event.target;
            var formData = new FormData(form);
            
            try {
                const response = await fetch('/update_autolock', {
                    method: 'POST',
                    body: formData
                });
                const data = await response.text();
                showPopup(data);
            } catch (error) {
                showPopup('Error: ' + error.message);
            }
        }

        // Bluetooth Functions with timeout
        async function fetchWithTimeout(url, options = {}, timeout = 5000) {
            const controller = new AbortController();
            const id = setTimeout(() => controller.abort(), timeout);
            
            try {
                const response = await fetch(url, {
                    ...options,
                    signal: controller.signal
                });
                clearTimeout(id);
                return response;
            } catch (error) {
                clearTimeout(id);
                if (error.name === 'AbortError') {
                    throw new Error('Request timed out');
                }
                throw error;
            }
        }

        async function updateDevices() {
            try {
                const response = await fetchWithTimeout('/devices');
                if (!response.ok) throw new Error('Network response was not ok');
                
                const data = await response.json();
                let html = '';
                
                if (data.length === 0) {
                    html = '<p>No bonded devices found.</p>';
                } else {
                    data.forEach((device, index) => {
                        let signalClass = 'poor';
                        if (device.rssi > -50) signalClass = 'good';
                        else if (device.rssi > -70) signalClass = 'medium';
                        
                        let signalText = device.rssi === -99 ? 'No Data' : device.rssi + ' dBm';
                        let distanceText = device.distance === 0 ? 'Unknown' : '~' + device.distance + 'm';
                        
                        html += '<div class="device ' + (device.priority ? 'priority' : '') + '">';
                        html += '<h3>' + (device.name || 'Unknown Device') + '</h3>';
                        html += '<p><strong>MAC:</strong> ' + device.mac + '</p>';
                        html += '<p><strong>Signal:</strong> <span class="signal ' + signalClass + '">' + signalText + '</span></p>';
                        html += '<p><strong>Distance:</strong> ' + distanceText + '</p>';
                        
                        html += '<div>';
                        html += '<input type="text" class="name-input" id="name_' + index + '" placeholder="Enter device name" value="' + (device.name || '') + '">';
                        html += '<button onclick="setDeviceName(\'' + device.mac + '\', ' + index + ')" class="button">Set Name</button>';
                        html += '</div>';
                        
                        html += '<div class="device-actions">';
                        html += '<button onclick="togglePriority(\'' + device.mac + '\')" class="button">';
                        html += device.priority ? 'Remove Priority' : 'Set Priority';
                        html += '</button>';
                        html += '<button onclick="removeDevice(\'' + device.mac + '\')" class="button remove">Remove Device</button>';
                        html += '</div>';
                        html += '</div>';
                    });
                }
                document.getElementById('devices').innerHTML = html;
            } catch (error) {
                document.getElementById('devices').innerHTML = '<p>Error loading devices: ' + error.message + '</p>';
            }
        }

        async function togglePairing() {
            try {
                const response = await fetchWithTimeout('/pairing');
                if (!response.ok) throw new Error('Network response was not ok');
                
                const status = await response.text();
                document.getElementById('pairingStatus').textContent = status;
            } catch (error) {
                document.getElementById('pairingStatus').textContent = 'Error: ' + error.message;
            }
        }

        async function setDeviceName(mac, index) {
            const nameInput = document.getElementById('name_' + index);
            const name = nameInput.value.trim();
            
            if (!name) {
                alert('Please enter a device name');
                return;
            }
            
            try {
                const response = await fetchWithTimeout('/setname', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ mac, name })
                });
                
                if (!response.ok) throw new Error('Network response was not ok');
                
                showPopup('Device name updated successfully');
                await updateDevices();
            } catch (error) {
                alert('Error setting device name: ' + error.message);
            }
        }

        async function togglePriority(mac) {
            try {
                const response = await fetchWithTimeout('/priority', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ mac })
                });
                
                if (!response.ok) throw new Error('Network response was not ok');
                
                showPopup('Priority updated successfully');
                await updateDevices();
            } catch (error) {
                alert('Error toggling priority: ' + error.message);
            }
        }

        async function removeDevice(mac) {
            if (!confirm('Are you sure you want to remove this device?')) {
                return;
            }
            
            try {
                const response = await fetchWithTimeout('/remove', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ mac })
                });
                
                if (!response.ok) throw new Error('Network response was not ok');
                
                showPopup('Device removed successfully');
                await updateDevices();
            } catch (error) {
                alert('Error removing device: ' + error.message);
            }
        }

        // Initialize page
        document.addEventListener('DOMContentLoaded', () => {
            showTab('main');
            // Refresh data every 10 seconds
            setInterval(() => {
                if (document.getElementById('mainTabContent').style.display !== 'none') {
                    loadSystemStatus();
                } else {
                    updateDevices();
                }
            }, 10000);
        });
    </script>
</head>
<body>
    <div class="container">
        <div id="popup" class="popup"></div>
        <h1>Ghost Key Configuration</h1>
        
        <!-- Tab Navigation -->
        <div class="tabs">
            <button onclick="showTab('main')" class="button" id="mainTab">Main Config</button>
            <button onclick="showTab('bluetooth')" class="button button2" id="bluetoothTab">Bluetooth Devices</button>
        </div>
        
        <!-- Main Configuration Tab -->
        <div id="mainTabContent" class="tab-content">
            <div class="status">
                <h2>System Status</h2>
                <div class="status-grid">
                    <div class="status-item">
                        <strong>Current State:</strong> <span id="systemState">Loading...</span>
                    </div>
                    <div class="status-item">
                        <strong>RFID Status:</strong> <span id="rfidStatus">Loading...</span>
                    </div>
                    <div class="status-item">
                        <strong>Bluetooth Status:</strong> <span id="bluetoothStatus">Loading...</span>
                    </div>
                    <div class="status-item">
                        <strong>Stored Keys:</strong> <span id="storedKeys">Loading...</span>
                    </div>
                </div>
            </div>
            
            <div class="form-group">
                <h2>Starter Configuration</h2>
                <form onsubmit="updatePulseTime(event)">
                    <label for="pulse_time">Starter Crank Time (ms):</label>
                    <input type="number" id="pulse_time" name="pulse_time" min="100" max="3000" step="100" value="1500">
                    <button type="submit" class="button">Update</button>
                </form>
            </div>
            
            <div class="form-group">
                <h2>Security Configuration</h2>
                <form onsubmit="updateAutoLock(event)">
                    <label for="auto_lock">Auto-Lock Timeout (ms):</label>
                    <input type="number" id="auto_lock" name="auto_lock" min="5000" max="120000" step="1000" value="30000">
                    <button type="submit" class="button">Update</button>
                </form>
            </div>
        </div>
        
        <!-- Bluetooth Devices Tab -->
        <div id="bluetoothTabContent" class="tab-content" style="display: none;">
            <h2>Bluetooth Device Management</h2>
            <div class="status">
                <button onclick="togglePairing()" class="button">Toggle Pairing Mode</button>
                <p id="pairingStatus">Pairing mode: inactive</p>
                <p><small>To pair a new device: Enable pairing mode, then make your device discoverable and connect to "Ghost-Key Secure"</small></p>
            </div>
            <div id="devices">Loading devices...</div>
        </div>
        
        <!-- Exit Config Mode -->
        <div style="margin-top: 30px; padding-top: 20px; border-top: 2px solid #eee;">
            <form action="/exit" method="post">
                <button type="submit" class="button button2">Exit Config Mode</button>
            </form>
        </div>
    </div>
</body>
</html>
)rawliteral";

#endif // CONFIG_HTML_H 