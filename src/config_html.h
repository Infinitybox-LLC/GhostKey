#ifndef CONFIG_HTML_H
#define CONFIG_HTML_H

const char config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>Ghost Key Configuration</title>
    
    <style>
        /* ======================================== */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Cantarell', sans-serif;
            background:
  radial-gradient(circle at 25% 30%, rgba(24, 53, 181, 0.3) 0%, rgba(102, 126, 234, 0) 70%),
  radial-gradient(circle at 70% 60%, rgba(102, 126, 234, 0.3) 0%, rgba(102, 126, 234, 0) 60%),
  radial-gradient(circle at 45% 80%, rgba(102, 126, 234, 0.28) 0%, rgba(102, 126, 234, 0) 65%),
  linear-gradient(135deg, #764ba2 0%,rgb(81, 55, 132) 100%);

            min-height: 100vh;
            color: #333;
            overflow-x: hidden;
            font-size: 16px;
            line-height: 1.6;
        }
        
        /* Typography - System fonts */
        .splash-title {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', sans-serif;
            font-weight: normal;
            letter-spacing: -0.03em;
        }
        
        .app-name, .card-title, .device-name, 
        h1, h2, h3, h4, h5, h6 {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', sans-serif;
            font-weight: normal;
            letter-spacing: -0.02em;
        }
        
        .form-label, .status-label {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', sans-serif;
            font-weight: normal;
        }
        
        .nav-item {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', sans-serif;
            font-weight: normal;
        }
        
        p, span, input, button, .btn {
            font-weight: 400;
        }
        
        /* Splash Screen Styles */
        .splash-screen {
            position: fixed;
            top: 0; left: 0; right: 0; bottom: 0;
            background:
  radial-gradient(circle at 25% 30%, rgba(102, 126, 234, 0.25) 0%, rgba(102, 126, 234, 0) 70%),
  radial-gradient(circle at 70% 60%, rgba(102, 126, 234, 0.2) 0%, rgba(102, 126, 234, 0) 60%),
  radial-gradient(circle at 45% 80%, rgba(102, 126, 234, 0.18) 0%, rgba(102, 126, 234, 0) 65%),
  linear-gradient(135deg, #764ba2 0%, #5a3c94 100%);
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            z-index: 9999;
            color: white;
        }
        
        .splash-logo {
            width: 200px;
            height: 200px;
            margin-bottom: 0rem;
            border-radius: 24px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.0);
        }
        
        .splash-title {
            font-size: 2rem;
            font-weight: 700;
            margin-bottom: 0.5rem;
            text-align: center;
        }
        
        .splash-subtitle {
            font-size: 1rem;
            opacity: 0.8;
            margin-bottom: 3rem;
            text-align: center;
        }
        
        .login-form {
            background: rgba(255,255,255,0.1);
            backdrop-filter: blur(10px);
            padding: 2rem;
            border-radius: 20px;
            min-width: 320px;
            text-align: center;
        }
        
        .login-input {
            width: 100%;
            padding: 1rem;
            font-size: 1.1rem;
            border: none;
            border-radius: 12px;
            background: rgba(255,255,255,0.9);
            margin-bottom: 1rem;
            text-align: center;
            letter-spacing: 2px;
        }
        
        .login-input:focus {
            outline: none;
            box-shadow: 0 0 0 3px rgba(255,255,255,0.3);
        }
        
        .login-btn {
            width: 100%;
            padding: 1rem;
            font-size: 1.1rem;
            font-weight: 600;
            border: none;
            border-radius: 12px;
            background: rgba(255,255,255,0.2);
            color: white;
            cursor: pointer;
            transition: all 0.3s;
            backdrop-filter: blur(10px);
        }
        
        .login-btn:hover {
            background: rgba(255,255,255,0.3);
            transform: translateY(-2px);
        }
        
        .login-error {
            color: #ff6b6b;
            margin-top: 1rem;
            font-weight: 500;
        }
        
        /* Main App Styles */
        .app-container {
            display: none;
            min-height: 100vh;
        }
        
        .app-header {
            background: rgba(255,255,255,0.95);
            backdrop-filter: blur(20px);
            padding: 0.75rem 2rem;
            position: sticky;
            top: 0;
            z-index: 100;
            box-shadow: 0 2px 20px rgba(0,0,0,0.1);
        }
        
        .app-title {
            display: flex;
            align-items: center;
            gap: 1rem;
        }
        
        .app-logo {
            width: 56px;
            height: 56px;
            border-radius: 10px;
        }
        
        .app-name {
            font-size: 1.5rem;
            font-weight: 700;
            color:rgb(119, 88, 185);
        }
        
        .main-container {
            display: flex;
            max-width: 1200px;
            margin: 0 auto;
            min-height: calc(100vh - 80px);
        }
        
        /* Sidebar Navigation */
        .sidebar {
            width: 280px;
            background: rgba(255,255,255,0.95);
            backdrop-filter: blur(20px);
            margin: 2rem 0 2rem 2rem;
            border-radius: 20px;
            padding: 2rem 0;
            box-shadow: 0 10px 40px rgba(0,0,0,0.1);
            height: fit-content;
            position: sticky;
            top: 100px;
        }
        
        .nav-item {
            display: flex;
            align-items: center;
            gap: 1rem;
            padding: 1rem 2rem;
            cursor: pointer;
            transition: all 0.3s;
            color: #666;
            font-size: 1.05rem;
            font-weight: 600;
        }
        
        .nav-item.active {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
            margin: 0 1rem;
            border-radius: 12px;
        }
        
        .nav-item:hover:not(.active) {
            background: rgba(102, 126, 234, 0.1);
            color: #667eea;
        }
        
        .nav-icon {
            width: 24px;
            height: 24px;
            font-size: 1.2rem;
        }
        
        /* Content Area */
        .content {
            flex: 1;
            padding: 2rem;
        }
        
        .content-section {
            display: none;
            animation: fadeIn 0.3s ease-in-out;
        }
        
        .content-section.active {
            display: block;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .card {
            background: rgba(255,255,255,0.95);
            backdrop-filter: blur(20px);
            border-radius: 20px;
            padding: 2rem;
            margin-bottom: 2rem;
            box-shadow: 0 10px 40px rgba(0,0,0,0.1);
        }
        
        .card-title {
            font-size: 1.5rem;
            font-weight: 700;
            color: #333;
            margin-bottom: 1.5rem;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }
        
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 1rem;
            margin-bottom: 2rem;
        }
        
        .status-item {
            background: linear-gradient(135deg, #f8f9ff, #e8edff);
            padding: 1.5rem;
            border-radius: 16px;
            border-left: 4px solid #667eea;
        }
        
        .status-label {
            font-size: 1rem;
            color: #666;
            margin-bottom: 0.5rem;
        }
        
        .status-value {
            font-size: 1.35rem;
            font-weight: 600;
            color: #333;
        }
        
        /* Form Elements */
        .form-group {
            margin-bottom: 2rem;
        }
        
        .form-label {
            display: block;
            font-weight: 600;
            color: #333;
            margin-bottom: 0.5rem;
        }
        
        .form-input {
            width: 100%;
            max-width: 300px;
            padding: 1rem;
            font-size: 1.1rem;
            border: 2px solid #e1e5e9;
            border-radius: 12px;
            background: white;
            transition: all 0.3s;
        }
        
        .form-input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        
        /* Buttons */
        .btn {
            padding: 1rem 2rem;
            font-size: 1.1rem;
            font-weight: 600;
            border: none;
            border-radius: 12px;
            cursor: pointer;
            transition: all 0.3s;
            text-decoration: none;
            display: inline-block;
            text-align: center;
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
        }
        
        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(102, 126, 234, 0.3);
        }
        
        .btn-secondary {
            background: #f8f9fa;
            color: #666;
            border: 2px solid #e1e5e9;
        }
        
        .btn-secondary:hover {
            background: #e9ecef;
            border-color: #667eea;
        }
        
        .btn-danger {
            background: linear-gradient(135deg, #ff6b6b, #ee5a52);
            color: white;
        }
        
        .btn-danger:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(255, 107, 107, 0.3);
        }
        
        /* Device Cards */
        .device-card {
            background: white;
            border-radius: 16px;
            padding: 1.5rem;
            margin-bottom: 1rem;
            box-shadow: 0 4px 20px rgba(0,0,0,0.08);
            transition: all 0.3s;
        }
        
        .device-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 30px rgba(0,0,0,0.12);
        }
        
        .device-header {
            display: flex;
            justify-content: space-between;
            align-items: flex-start;
            margin-bottom: 1rem;
        }
        
        .device-name {
            font-size: 1.35rem;
            font-weight: 600;
            color: #333;
        }
        
        .device-mac {
            font-size: 1rem;
            color: #666;
            font-family: monospace;
        }
        
        .signal-indicator {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            margin: 0.5rem 0;
        }
        
        .signal-light {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            box-shadow: 0 0 0 2px rgba(255,255,255,0.3);
            animation: pulse 2s infinite;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.6; }
        }
        
        .signal-good { background: #28a745; }
        .signal-medium { background: #ffc107; }
        .signal-poor { background: #dc3545; }
        
        .device-actions {
            display: flex;
            gap: 0.5rem;
            flex-wrap: wrap;
            margin-top: 1rem;
        }
        
        .device-actions .btn {
            padding: 0.5rem 1rem;
            font-size: 1rem;
        }
        
        /* Toggle Switch */
        .toggle-switch {
            position: relative;
            width: 60px;
            height: 34px;
        }
        
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: #ccc;
            transition: 0.4s;
            border-radius: 34px;
        }
        
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: 0.4s;
            border-radius: 50%;
        }
        
        input:checked + .slider {
            background: linear-gradient(135deg, #667eea, #764ba2);
        }
        
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        
        /* Notifications */
        .notification {
            position: fixed;
            top: 2rem;
            right: 2rem;
            background: white;
            color: #333;
            padding: 1rem 2rem;
            border-radius: 12px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.15);
            display: none;
            z-index: 1000;
            animation: slideIn 0.3s ease-out;
        }
        
                 @keyframes slideIn {
             from { transform: translateX(100%); }
             to { transform: translateX(0); }
         }
         
         @keyframes shake {
             0%, 100% { transform: translateX(0); }
             25% { transform: translateX(-10px); }
             75% { transform: translateX(10px); }
         }
        
        .notification.success {
            border-left: 4px solid #28a745;
        }
        
        .notification.error {
            border-left: 4px solid #dc3545;
        }
        
        /* Exit Button Styling */
        .exit-nav {
            color: #dc3545 !important;
            font-weight: 600;
        }
        
        .exit-nav:hover:not(.active) {
            background: linear-gradient(135deg, #dc3545, #c82333) !important;
            color: white !important;
        }
        
        /* Exit Splash Screen */
        .exit-splash-screen {
            position: fixed;
            top: 0; left: 0; right: 0; bottom: 0;
            background: linear-gradient(135deg, #28a745 0%, #20c997 100%);
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            z-index: 10000;
            color: white;
        }
        
        .exit-message {
            background: rgba(255,255,255,0.1);
            backdrop-filter: blur(10px);
            padding: 2rem;
            border-radius: 20px;
            text-align: center;
            margin-top: 2rem;
        }
        
        .exit-message p {
            margin: 0.5rem 0;
            font-size: 1.2rem;
        }
        
        /* Mobile Responsive */
        @media (max-width: 768px) {
            .main-container {
                flex-direction: column;
            }
            
            .sidebar {
                width: 100%;
                margin: 1rem;
                position: static;
                display: flex;
                overflow-x: auto;
                padding: 1rem 0;
            }
            
            .nav-item {
                min-width: 120px;
                justify-content: center;
                padding: 1rem;
                margin: 0 0.5rem;
                border-radius: 12px;
                text-align: center;
            }
            
            .nav-item.active {
                margin: 0 0.5rem;
            }
            
            .content {
                padding: 1rem;
            }
            
            .card {
                padding: 1.5rem;
            }
            
            .status-grid {
                grid-template-columns: 1fr;
            }
            
            .device-actions {
                flex-direction: column;
            }
            
            .device-actions .btn {
                width: 100%;
            }
            
            .app-header {
                padding: 0.75rem 1rem;
            }
            
            .notification {
                top: 1rem;
                right: 1rem;
                left: 1rem;
            }
        }
        
        @media (max-width: 480px) {
            .login-form {
                min-width: 280px;
                padding: 1.5rem;
            }
            
            .splash-title {
                font-size: 1.5rem;
            }
            
            .form-input {
                max-width: 100%;
            }
        }
    </style>
    <script>
        // Configuration
        let appPassword = localStorage.getItem('ghostkey_password') || '1234';
        let isAuthenticated = false;
        
        // Utility Functions
        function showNotification(message, type = 'success') {
            const notification = document.getElementById('notification');
            notification.textContent = message;
            notification.className = `notification ${type}`;
            notification.style.display = 'block';
            setTimeout(() => { 
                notification.style.display = 'none'; 
            }, 5000);
        }

        function showSection(sectionName) {
            // Hide all sections
            document.querySelectorAll('.content-section').forEach(section => {
                section.classList.remove('active');
            });
            
            // Remove active class from all nav items
            document.querySelectorAll('.nav-item').forEach(item => {
                item.classList.remove('active');
            });
            
            // Show selected section
            const section = document.getElementById(sectionName + 'Section');
            const navItem = document.getElementById(sectionName + 'Nav');
            
            if (section && navItem) {
                section.classList.add('active');
                navItem.classList.add('active');
                
                // Load data for the section
                if (sectionName === 'overview') {
                    loadSystemStatus();
                } else if (sectionName === 'bluetooth') {
                    loadBluetoothStatus();
                    updateDevices();
                } else if (sectionName === 'rfid') {
                    loadRfidKeys();
                } else if (sectionName === 'security') {
                    loadSystemConfig();
                    loadWifiPassword();
                }
            }
        }

        // Authentication Functions
        function checkAuth() {
            if (!isAuthenticated) {
                document.querySelector('.splash-screen').style.display = 'flex';
                document.querySelector('.app-container').style.display = 'none';
            } else {
                document.querySelector('.splash-screen').style.display = 'none';
                document.querySelector('.app-container').style.display = 'block';
                showSection('overview');
            }
        }

        function handleLogin(event) {
            event.preventDefault();
            const input = document.getElementById('passwordInput');
            const password = input.value;
            const errorDiv = document.getElementById('loginError');
            
            if (password === appPassword) {
                isAuthenticated = true;
                checkAuth();
                input.value = '';
                errorDiv.textContent = '';
            } else {
                errorDiv.textContent = 'Invalid password';
                input.value = '';
                // Shake animation
                input.style.animation = 'shake 0.5s';
                setTimeout(() => input.style.animation = '', 500);
            }
        }

        function logout() {
            isAuthenticated = false;
            checkAuth();
        }

        // System Status Functions
        async function loadSystemStatus() {
            try {
                const response = await fetch('/status');
                if (response.ok) {
                    const data = await response.json();
                    document.getElementById('rfidStatus').textContent = data.rfid ? 'Authenticated' : 'Not Authenticated';
                    document.getElementById('bluetoothStatus').textContent = data.bluetooth ? 'Authenticated' : 'Not Authenticated';
                    document.getElementById('storedKeys').textContent = data.keys || '0';
                    
                    // Update form values - convert from milliseconds to seconds
                    if (data.starterPulse) {
                        document.getElementById('pulseTime').value = (data.starterPulse / 1000).toFixed(1);
                    }
                    if (data.autoLockTimeout) {
                        document.getElementById('autoLockTime').value = Math.round(data.autoLockTimeout / 1000);
                    }
                }
                
                // Load system configuration status
                await loadSystemConfig();
                
            } catch (error) {
                console.error('Error loading system status:', error);
                showNotification('Error loading system status', 'error');
            }
        }

        // Ghost Key/Power system management
        async function loadSystemConfig() {
            try {
                const response = await fetch('/system_status');
                if (response.ok) {
                    const data = await response.json();
                    document.getElementById('ghostKeyEnabled').checked = data.ghostKeyEnabled;
                    document.getElementById('ghostPowerEnabled').checked = data.ghostPowerEnabled;
                    updateUIVisibility(data.ghostKeyEnabled, data.ghostPowerEnabled);
                }
            } catch (error) {
                console.error('Error loading system configuration:', error);
            }
        }

        async function toggleGhostKey() {
            const enabled = document.getElementById('ghostKeyEnabled').checked;
            const ghostPowerEnabled = document.getElementById('ghostPowerEnabled').checked;
            
            // Prevent disabling both systems
            if (!enabled && !ghostPowerEnabled) {
                document.getElementById('ghostKeyEnabled').checked = true;
                showNotification('Cannot disable both Ghost Key and Ghost Power systems', 'error');
                return;
            }
            
            if (!enabled && !confirm('Are you sure you want to disable Ghost Key? This will remove RFID/Bluetooth authentication and push-to-start functionality.')) {
                document.getElementById('ghostKeyEnabled').checked = true;
                return;
            }
            
            try {
                const response = await fetch('/toggle_ghost_key', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ enabled: enabled })
                });
                
                if (!response.ok) {
                    const errorText = await response.text();
                    throw new Error(errorText);
                }
                
                const message = enabled ? 'Ghost Key enabled.' : 'Ghost Key disabled.';
                showNotification(message);
                updateUIVisibility(enabled, ghostPowerEnabled);
                
            } catch (error) {
                document.getElementById('ghostKeyEnabled').checked = !enabled;
                showNotification('Error: ' + error.message, 'error');
            }
        }

        async function toggleGhostPower() {
            const enabled = document.getElementById('ghostPowerEnabled').checked;
            const ghostKeyEnabled = document.getElementById('ghostKeyEnabled').checked;
            
            // Prevent disabling both systems
            if (!enabled && !ghostKeyEnabled) {
                document.getElementById('ghostPowerEnabled').checked = true;
                showNotification('Cannot disable both Ghost Key and Ghost Power systems', 'error');
                return;
            }
            
            if (!enabled && !confirm('Are you sure you want to disable Ghost Power? This will remove security relay functionality.')) {
                document.getElementById('ghostPowerEnabled').checked = true;
                return;
            }
            
            try {
                const response = await fetch('/toggle_ghost_power', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ enabled: enabled })
                });
                
                if (!response.ok) {
                    const errorText = await response.text();
                    throw new Error(errorText);
                }
                
                const message = enabled ? 'Ghost Power enabled.' : 'Ghost Power disabled.';
                showNotification(message);
                updateUIVisibility(ghostKeyEnabled, enabled);
                
            } catch (error) {
                document.getElementById('ghostPowerEnabled').checked = !enabled;
                showNotification('Error: ' + error.message, 'error');
            }
        }

        function updateUIVisibility(ghostKeyEnabled, ghostPowerEnabled) {
            // Navigation visibility
            const configNav = document.getElementById('configNav');
            const bluetoothNav = document.getElementById('bluetoothNav');
            const rfidNav = document.getElementById('rfidNav');
            
            if (ghostKeyEnabled) {
                // Show full interface for Ghost Key
                configNav.style.display = 'flex';
                bluetoothNav.style.display = 'flex';
                rfidNav.style.display = 'flex';
            } else {
                // Hide Ghost Key sections if only Ghost Power
                configNav.style.display = 'none';
                bluetoothNav.style.display = 'none';
                rfidNav.style.display = 'none';
                
                // If we're currently viewing a hidden section, switch to security
                const activeSection = document.querySelector('.content-section.active');
                if (activeSection && (activeSection.id === 'configSection' || 
                                    activeSection.id === 'bluetoothSection' || 
                                    activeSection.id === 'rfidSection')) {
                    showSection('security');
                }
            }
        }

        // Configuration Functions
        async function updatePulseTime() {
            const pulseTimeSeconds = document.getElementById('pulseTime').value;
            const pulseTimeMs = Math.round(pulseTimeSeconds * 1000); // Convert seconds to milliseconds
            const formData = new FormData();
            formData.append('pulse_time', pulseTimeMs);
            
            try {
                const response = await fetch('/update_pulse', {
                    method: 'POST',
                    body: formData
                });
                const data = await response.text();
                showNotification(data);
            } catch (error) {
                showNotification('Error: ' + error.message, 'error');
            }
        }

        async function updateAutoLock() {
            const autoLockSeconds = document.getElementById('autoLockTime').value;
            const autoLockMs = autoLockSeconds * 1000; // Convert seconds to milliseconds
            const formData = new FormData();
            formData.append('auto_lock', autoLockMs);
            
            try {
                const response = await fetch('/update_autolock', {
                    method: 'POST',
                    body: formData
                });
                const data = await response.text();
                showNotification(data);
            } catch (error) {
                showNotification('Error: ' + error.message, 'error');
            }
        }

        // Password Management
        async function updateWebPassword() {
            const newPassword = document.getElementById('newPassword').value;
            const confirmPassword = document.getElementById('confirmPassword').value;
            
            if (newPassword.length < 4) {
                showNotification('Web interface password must be at least 4 characters', 'error');
                return;
            }
            
            if (newPassword !== confirmPassword) {
                showNotification('Web interface passwords do not match', 'error');
                return;
            }
            
            localStorage.setItem('ghostkey_password', newPassword);
            appPassword = newPassword;
            document.getElementById('newPassword').value = '';
            document.getElementById('confirmPassword').value = '';
            showNotification('Web interface password updated successfully');
        }

        async function loadWifiPassword() {
            try {
                const response = await fetchWithTimeout('/wifi_password');
                if (response.ok) {
                    const data = await response.json();
                    document.getElementById('currentWifiPassword').value = data.password || '';
                }
            } catch (error) {
                console.error('Error loading WiFi password:', error);
                document.getElementById('currentWifiPassword').value = 'Error loading';
            }
        }

        async function updateWifiPassword() {
            const newPassword = document.getElementById('newWifiPassword').value;
            const confirmPassword = document.getElementById('confirmWifiPassword').value;
            
            if (newPassword.length < 8) {
                showNotification('WiFi password must be at least 8 characters', 'error');
                return;
            }
            
            if (newPassword !== confirmPassword) {
                showNotification('WiFi passwords do not match', 'error');
                return;
            }
            
            try {
                const response = await fetchWithTimeout('/update_wifi_password', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ password: newPassword })
                });
                
                if (!response.ok) throw new Error('Network response was not ok');
                
                document.getElementById('newWifiPassword').value = '';
                document.getElementById('confirmWifiPassword').value = '';
                showNotification('WiFi password saved. Changes will take effect when you exit configuration mode.');
                
                // Update the current password display
                document.getElementById('currentWifiPassword').value = newPassword;
                
            } catch (error) {
                showNotification('Error updating WiFi password: ' + error.message, 'error');
            }
        }

        // Bluetooth Enable/Disable Functions
        async function loadBluetoothStatus() {
            try {
                const response = await fetchWithTimeout('/bluetooth_status');
                if (response.ok) {
                    const data = await response.json();
                    document.getElementById('bluetoothEnabled').checked = data.enabled;
                    updateBluetoothUI(data.enabled);
                }
            } catch (error) {
                console.error('Error loading Bluetooth status:', error);
            }
        }

        async function toggleBluetoothEnabled() {
            const enabled = document.getElementById('bluetoothEnabled').checked;
            
            try {
                const response = await fetchWithTimeout('/bluetooth_toggle', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ enabled: enabled })
                });
                
                if (!response.ok) throw new Error('Network response was not ok');
                
                const message = enabled ? 'Bluetooth enabled.' : 'Bluetooth disabled.';
                showNotification(message);
                updateBluetoothUI(enabled);
                
            } catch (error) {
                // Revert toggle on error
                document.getElementById('bluetoothEnabled').checked = !enabled;
                showNotification('Error updating Bluetooth setting: ' + error.message, 'error');
            }
        }

        function updateBluetoothUI(enabled) {
            const pairingButton = document.querySelector('button[onclick="togglePairing()"]');
            const devicesList = document.getElementById('devicesList');
            
            if (enabled) {
                pairingButton.disabled = false;
                pairingButton.style.opacity = '1';
                if (devicesList) {
                    devicesList.style.opacity = '1';
                    devicesList.style.pointerEvents = 'auto';
                }
            } else {
                pairingButton.disabled = true;
                pairingButton.style.opacity = '0.5';
                if (devicesList) {
                    devicesList.style.opacity = '0.5';
                    devicesList.style.pointerEvents = 'none';
                    devicesList.innerHTML = '<div class="card"><p style="color: #888;">Bluetooth is disabled. Enable Bluetooth above to manage devices.</p></div>';
                }
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
            // Check if Bluetooth is enabled first
            const bluetoothToggle = document.getElementById('bluetoothEnabled');
            if (bluetoothToggle && !bluetoothToggle.checked) {
                document.getElementById('devicesList').innerHTML = '<div class="card"><p style="color: #888;">Bluetooth is disabled. Enable Bluetooth above to manage devices.</p></div>';
                return;
            }
            
            try {
                const response = await fetchWithTimeout('/devices');
                if (!response.ok) throw new Error('Network response was not ok');
                
                const data = await response.json();
                let html = '';
                
                if (data.length === 0) {
                    html = '<div class="card"><p>No bonded devices found.</p></div>';
                } else {
                    data.forEach((device, index) => {
                        let signalClass = 'signal-poor';
                        if (device.rssi > -50) signalClass = 'signal-good';
                        else if (device.rssi > -70) signalClass = 'signal-medium';
                        
                        html += '<div class="device-card">';
                        html += '<div class="device-header">';
                        html += '<div>';
                        html += '<div class="device-name">' + (device.name || 'Unknown Device') + '</div>';
                        html += '<div class="device-mac">' + device.mac + '</div>';
                        html += '</div>';
                        html += '<div class="signal-indicator">';
                        html += '<div class="signal-light ' + signalClass + '" title="Signal: ' + (device.rssi === -99 ? 'No Data' : device.rssi + ' dBm') + '"></div>';
                        html += '</div>';
                        html += '</div>';
                        
                        html += '<div>';
                        html += '<input type="text" class="form-input" id="name_' + index + '" placeholder="Enter device name" value="' + (device.name || '') + '" style="margin-bottom: 1rem;">';
                        html += '</div>';
                        
                        html += '<div class="device-actions">';
                        html += '<button onclick="setDeviceName(\'' + device.mac + '\', ' + index + ')" class="btn btn-primary">Set Name</button>';
                        html += '<button onclick="togglePriority(\'' + device.mac + '\')" class="btn btn-secondary">';
                        html += device.priority ? 'Remove Priority' : 'Set Priority';
                        html += '</button>';
                        html += '<button onclick="removeDevice(\'' + device.mac + '\')" class="btn btn-danger">Remove</button>';
                        html += '</div>';
                        html += '</div>';
                    });
                }
                document.getElementById('devicesList').innerHTML = html;
            } catch (error) {
                document.getElementById('devicesList').innerHTML = '<div class="card"><p>Error loading devices: ' + error.message + '</p></div>';
            }
        }

        async function togglePairing() {
            try {
                const response = await fetchWithTimeout('/pairing');
                if (!response.ok) throw new Error('Network response was not ok');
                
                const status = await response.text();
                document.getElementById('pairingStatus').textContent = status;
                showNotification(status);
            } catch (error) {
                const errorMsg = 'Error: ' + error.message;
                document.getElementById('pairingStatus').textContent = errorMsg;
                showNotification(errorMsg, 'error');
            }
        }

        async function setDeviceName(mac, index) {
            const nameInput = document.getElementById('name_' + index);
            const name = nameInput.value.trim();
            
            if (!name) {
                showNotification('Please enter a device name', 'error');
                return;
            }
            
            try {
                const response = await fetchWithTimeout('/setname', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ mac, name })
                });
                
                if (!response.ok) throw new Error('Network response was not ok');
                
                showNotification('Device name updated successfully');
                await updateDevices();
            } catch (error) {
                showNotification('Error setting device name: ' + error.message, 'error');
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
                
                showNotification('Priority updated successfully');
                await updateDevices();
            } catch (error) {
                showNotification('Error toggling priority: ' + error.message, 'error');
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
                
                showNotification('Device removed successfully');
                await updateDevices();
            } catch (error) {
                showNotification('Error removing device: ' + error.message, 'error');
            }
        }

        // RFID Management Functions
        async function loadRfidKeys() {
            try {
                const response = await fetchWithTimeout('/rfid_keys');
                if (!response.ok) throw new Error('Network response was not ok');
                
                const data = await response.json();
                
                // Get RFID status separately
                const statusResponse = await fetchWithTimeout('/rfid_status');
                if (statusResponse.ok) {
                    const statusData = await statusResponse.json();
                    document.getElementById('rfidPairingStatus').textContent = statusData.pairing ? 'Pairing mode: active' : 'Pairing mode: inactive';
                }
                
                let html = '';
                if (data.length === 0) {
                    html = '<div class="card"><p>No RFID keys found.</p></div>';
                } else {
                    data.forEach((key, index) => {
                        html += '<div class="device-card">';
                        html += '<div class="device-header">';
                        html += '<div>';
                        html += '<div class="device-name">' + (key.name || 'RFID Key #' + (index + 1)) + '</div>';
                        html += '<div class="device-mac">' + key.id + '</div>';
                        html += '</div>';
                        html += '</div>';
                        
                        html += '<div class="device-actions">';
                        html += '<button onclick="removeRfidKey(' + index + ')" class="btn btn-danger">Remove Key</button>';
                        html += '</div>';
                        html += '</div>';
                    });
                }
                document.getElementById('rfidKeysList').innerHTML = html;
            } catch (error) {
                document.getElementById('rfidKeysList').innerHTML = '<div class="card"><p>Error loading RFID keys: ' + error.message + '</p></div>';
            }
        }

        async function toggleRfidPairing() {
            try {
                const response = await fetchWithTimeout('/rfid_pair', { method: 'POST' });
                if (!response.ok) throw new Error('Network response was not ok');
                
                const status = await response.text();
                document.getElementById('rfidPairingStatus').textContent = 'Pairing mode: active';
                showNotification(status);
            } catch (error) {
                const errorMsg = 'Error: ' + error.message;
                document.getElementById('rfidPairingStatus').textContent = errorMsg;
                showNotification(errorMsg, 'error');
            }
        }



        async function removeRfidKey(index) {
            if (!confirm('Are you sure you want to remove this RFID key?')) {
                return;
            }
            
            try {
                const response = await fetchWithTimeout('/rfid_remove', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ index: index })
                });
                
                if (!response.ok) throw new Error('Network response was not ok');
                
                showNotification('RFID key removed successfully');
                await loadRfidKeys();
            } catch (error) {
                showNotification('Error removing key: ' + error.message, 'error');
            }
        }

        async function exitConfig() {
            try {
                // Show exit splash screen
                document.querySelector('.exit-splash-screen').style.display = 'flex';
                document.querySelector('.app-container').style.display = 'none';
                
                const response = await fetch('/exit', { method: 'POST' });
                if (response.ok) {
                    showNotification('Configuration mode ended');
                    setTimeout(() => {
                        // Hide exit splash and show login
                        document.querySelector('.exit-splash-screen').style.display = 'none';
                        logout();
                    }, 3000);
                }
            } catch (error) {
                showNotification('Error exiting config mode: ' + error.message, 'error');
                document.querySelector('.exit-splash-screen').style.display = 'none';
                document.querySelector('.app-container').style.display = 'block';
            }
        }

        // Initialize page
        document.addEventListener('DOMContentLoaded', () => {
            checkAuth();
            
            // Load system configuration on page load to set UI visibility
            setTimeout(() => {
                if (isAuthenticated) {
                    loadSystemConfig();
                }
            }, 1000);
            
            // Refresh data every 15 seconds when authenticated
            setInterval(() => {
                if (isAuthenticated) {
                    const activeSection = document.querySelector('.content-section.active');
                    if (activeSection && activeSection.id === 'overviewSection') {
                        loadSystemStatus();
                    } else if (activeSection && activeSection.id === 'bluetoothSection') {
                        loadBluetoothStatus();
                        updateDevices();
                    } else if (activeSection && activeSection.id === 'rfidSection') {
                        loadRfidKeys();
                    } else if (activeSection && activeSection.id === 'securitySection') {
                        loadSystemConfig();
                        loadWifiPassword();
                    }
                }
            }, 15000);
        });
    </script>
</head>
<body>
    <!-- Splash Screen -->
    <div class="splash-screen">
        <img src="/logo" class="splash-logo" alt="Ghost Key Logo" onerror="this.style.display='none'">
        <h1 class="splash-title">Ghost Key</h1>
        <p class="splash-subtitle">Secure Vehicle Access System</p>
        
        <form class="login-form" onsubmit="handleLogin(event)">
            <input type="password" id="passwordInput" class="login-input" placeholder="Enter PIN or Password" autocomplete="off">
            <button type="submit" class="login-btn">Unlock</button>
            <div id="loginError" class="login-error"></div>
        </form>
    </div>

    <!-- Exit Splash Screen -->
    <div class="exit-splash-screen" style="display: none;">
        <img src="/logo" class="splash-logo" alt="Ghost Key Logo" onerror="this.style.display='none'">
        <h1 class="splash-title">Configuration Mode Ended</h1>
        <p class="splash-subtitle">Thank you for using Ghost Key</p>
        <div class="exit-message">
            <p>✓ Settings saved successfully</p>
            <p>Returning to normal operation...</p>
        </div>
    </div>

    <!-- Main Application -->
    <div class="app-container">
        <!-- Header -->
        <header class="app-header">
            <div class="app-title">
                <img src="/logo" class="app-logo" alt="Logo" onerror="this.style.display='none'">
                <span class="app-name">Ghost Key Configuration</span>
            </div>
        </header>

        <div class="main-container">
            <!-- Sidebar Navigation -->
            <nav class="sidebar">
                <div class="nav-item active" id="overviewNav" onclick="showSection('overview')">
                    <span>Overview</span>
                </div>
                <div class="nav-item" id="configNav" onclick="showSection('config')">
                    <span>Configuration</span>
                </div>
                <div class="nav-item" id="bluetoothNav" onclick="showSection('bluetooth')">
                    <span>Bluetooth</span>
                </div>
                <div class="nav-item" id="rfidNav" onclick="showSection('rfid')">
                    <span>RFID Keys</span>
                </div>
                <div class="nav-item" id="securityNav" onclick="showSection('security')">
                    <span>Security</span>
                </div>
                <div class="nav-item exit-nav" onclick="exitConfig()">
                    <span>Exit Configuration Mode</span>
                </div>
            </nav>

            <!-- Content Area -->
            <main class="content">
                <!-- Overview Section -->
                <section class="content-section active" id="overviewSection">
                    <div class="card">
                        <h2 class="card-title">System Status</h2>
                        <div class="status-grid">
                            <div class="status-item">
                                <div class="status-label">RFID Status</div>
                                <div class="status-value" id="rfidStatus">Loading...</div>
                            </div>
                            <div class="status-item">
                                <div class="status-label">Bluetooth Status</div>
                                <div class="status-value" id="bluetoothStatus">Loading...</div>
                            </div>
                            <div class="status-item">
                                <div class="status-label">Stored Keys</div>
                                <div class="status-value" id="storedKeys">Loading...</div>
                            </div>
                        </div>
                    </div>
                </section>

                <!-- Bluetooth Section -->
                <section class="content-section" id="bluetoothSection">
                    <div class="card">
                        <h2 class="card-title">Bluetooth Configuration</h2>
                        <div class="form-group" style="margin-bottom: 2rem;">
                            <div style="display: flex; align-items: center; gap: 1rem; margin-bottom: 1rem;">
                                <label class="form-label" for="bluetoothEnabled" style="margin-bottom: 0;">Enable Bluetooth Authentication</label>
                                <label class="toggle-switch">
                                    <input type="checkbox" id="bluetoothEnabled" onchange="toggleBluetoothEnabled()">
                                    <span class="slider"></span>
                                </label>
                            </div>
                            <p style="font-size: 0.9rem; color: #666; margin-bottom: 2rem;">
                                When disabled, Bluetooth authentication will be completely turned off to save power and improve security.
                            </p>
                        </div>
                    </div>
                    
                    <div class="card">
                        <h2 class="card-title">Bluetooth Device Management</h2>
                        <div style="margin-bottom: 2rem;">
                            <button onclick="togglePairing()" class="btn btn-primary">Toggle Pairing Mode</button>
                            <p id="pairingStatus" style="margin-top: 1rem; color: #666;">Pairing mode: inactive</p>
                            <p style="font-size: 0.9rem; color: #888; margin-top: 0.5rem;">
                                To pair a new device: Enable pairing mode, then connect to "Ghost-Key Secure" from your device's Bluetooth settings.
                            </p>
                        </div>
                    </div>
                    
                    <div id="devicesList">
                        <div class="card">Loading devices...</div>
                    </div>
                </section>

                <!-- Configuration Section -->
                <section class="content-section" id="configSection">
                    <div class="card">
                        <h2 class="card-title">Starter Configuration</h2>
                        <div class="form-group">
                            <label class="form-label" for="pulseTime">Starter Crank Time (seconds)</label>
                            <input type="number" id="pulseTime" class="form-input" min="0.1" max="3.0" step="0.1" value="0.7">
                            <button onclick="updatePulseTime()" class="btn btn-primary" style="margin-left: 1rem;">Update</button>
                        </div>
                    </div>
                    
                    <div class="card">
                        <h2 class="card-title">Security Configuration</h2>
                        <div class="form-group">
                            <label class="form-label" for="autoLockTime">Auto-Lock Timeout (seconds)</label>
                            <input type="number" id="autoLockTime" class="form-input" min="5" max="120" step="1" value="30">
                            <button onclick="updateAutoLock()" class="btn btn-primary" style="margin-left: 1rem;">Update</button>
                        </div>
                    </div>
                </section>

                <!-- RFID Section -->
                <section class="content-section" id="rfidSection">
                    <div class="card">
                        <h2 class="card-title">RFID Key Management</h2>
                        <div style="margin-bottom: 2rem;">
                            <button onclick="toggleRfidPairing()" class="btn btn-primary">Toggle RFID Pairing Mode</button>
                            <p id="rfidPairingStatus" style="margin-top: 1rem; color: #666;">Pairing mode: inactive</p>
                            <p style="font-size: 0.9rem; color: #888; margin-top: 0.5rem;">
                                To pair a new RFID key: Enable pairing mode, then hold your RFID tag near the reader.
                            </p>
                        </div>
                    </div>
                    
                    <div id="rfidKeysList">
                        <div class="card">Loading RFID keys...</div>
                    </div>
                </section>

                <!-- Security Section -->
                <section class="content-section" id="securitySection">
                    <div class="card">
                        <h2 class="card-title">System Configuration</h2>
                        <div class="form-group" style="margin-bottom: 2rem;">
                            <div style="display: flex; align-items: center; gap: 1rem; margin-bottom: 1rem;">
                                <label class="form-label" for="ghostKeyEnabled" style="margin-bottom: 0;">Enable Ghost Key</label>
                                <label class="toggle-switch">
                                    <input type="checkbox" id="ghostKeyEnabled" onchange="toggleGhostKey()">
                                    <span class="slider"></span>
                                </label>
                            </div>
                            <p style="font-size: 0.9rem; color: #666; margin-bottom: 2rem;">
                                RFID/Bluetooth authentication and push-to-start functionality.
                            </p>
                            
                            <div style="display: flex; align-items: center; gap: 1rem; margin-bottom: 1rem;">
                                <label class="form-label" for="ghostPowerEnabled" style="margin-bottom: 0;">Enable Ghost Power</label>
                                <label class="toggle-switch">
                                    <input type="checkbox" id="ghostPowerEnabled" onchange="toggleGhostPower()">
                                    <span class="slider"></span>
                                </label>
                            </div>
                            <p style="font-size: 0.9rem; color: #666; margin-bottom: 2rem;">
                                Security relay control and immobilization features.
                            </p>
                            
                            <p style="font-size: 0.9rem; color: #ff6b6b; font-weight: 600;">
                                ⚠️ At least one system must remain enabled
                            </p>
                        </div>
                    </div>
                    
                    <div class="card">
                        <h2 class="card-title">Web Interface Password</h2>
                        <div class="form-group">
                            <label class="form-label" for="newPassword">New Web Interface Password</label>
                            <input type="password" id="newPassword" class="form-input" placeholder="Enter new password" minlength="4">
                        </div>
                        <div class="form-group">
                            <label class="form-label" for="confirmPassword">Confirm Web Interface Password</label>
                            <input type="password" id="confirmPassword" class="form-input" placeholder="Confirm new password" minlength="4">
                        </div>
                        <button onclick="updateWebPassword()" class="btn btn-primary">Update Web Interface Password</button>
                        <p style="font-size: 0.9rem; color: #666; margin-top: 1rem;">
                            This password protects access to this configuration interface. Minimum 4 characters required.
                        </p>
                    </div>
                    
                    <div class="card">
                        <h2 class="card-title">WiFi Access Point Password</h2>
                        <div class="form-group">
                            <label class="form-label" for="currentWifiPassword">Current WiFi Password</label>
                            <input type="text" id="currentWifiPassword" class="form-input" placeholder="Loading..." readonly>
                        </div>
                        <div class="form-group">
                            <label class="form-label" for="newWifiPassword">New WiFi Password</label>
                            <input type="password" id="newWifiPassword" class="form-input" placeholder="Enter new WiFi password" minlength="8">
                        </div>
                        <div class="form-group">
                            <label class="form-label" for="confirmWifiPassword">Confirm WiFi Password</label>
                            <input type="password" id="confirmWifiPassword" class="form-input" placeholder="Confirm new WiFi password" minlength="8">
                        </div>
                        <button onclick="updateWifiPassword()" class="btn btn-primary">Update WiFi Password</button>
                        <p style="font-size: 0.9rem; color: #666; margin-top: 1rem;">
                            This password is required to connect to the "Ghost Key Configuration" WiFi network. Minimum 8 characters required.
                        </p>
                        <p style="font-size: 0.9rem; color: #28a745; margin-top: 0.5rem;">
                            💡 Changes will take effect when you exit configuration mode. You won't be disconnected while configuring.
                        </p>
                    </div>
                    
                    <div class="card">
                        <h2 class="card-title">Reset Options</h2>
                        <button onclick="logout()" class="btn btn-secondary">Lock Configuration</button>
                        <p style="font-size: 0.9rem; color: #666; margin-top: 1rem;">
                            Lock the configuration interface. You'll need to enter the password again to access settings.
                        </p>
                    </div>
                </section>
            </main>
        </div>
    </div>

    <!-- Notification -->
    <div id="notification" class="notification"></div>
</body>
</html>
)rawliteral";

#endif // CONFIG_HTML_H 