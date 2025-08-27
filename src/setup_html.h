#ifndef SETUP_HTML_H
#define SETUP_HTML_H

const char setup_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no, shrink-to-fit=no, viewport-fit=cover">
    <title>Ghost Key First Time Setup</title>
    
    <!-- PWA Manifest -->
    <link rel="manifest" href="/manifest.json">
    
    <!-- PWA Meta Tags -->
    <meta name="theme-color" content="#667eea">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black">
    <meta name="apple-mobile-web-app-title" content="Ghost Key">
    <meta name="apple-touch-fullscreen" content="yes">
    <meta name="mobile-web-app-capable" content="yes">
    <link rel="apple-touch-icon" href="/logo">
    <link rel="apple-touch-icon" sizes="152x152" href="/logo">
    <link rel="apple-touch-icon" sizes="167x167" href="/logo">
    <link rel="apple-touch-icon" sizes="180x180" href="/logo">
    
    <style>
        /* Base styling matching current interface */
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            max-width: 100%;
            word-wrap: break-word;
            overflow-wrap: break-word;
        }
        
        html {
            overflow-x: hidden;
            width: 100%;
            max-width: 100%;
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
            width: 100%;
            max-width: 100%;
            font-size: 16px;
            line-height: 1.6;
            padding: 2rem 0;
            position: relative;
        }
        
        .setup-container {
            max-width: 900px;
            width: 100%;
            margin: 0 auto;
            padding: 0 2rem;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            gap: 3rem;
            overflow-x: hidden;
            position: relative;
        }
        
        @media (max-width: 950px) {
            .setup-container {
                max-width: 100vw;
                padding: 0 1.5rem;
            }
        }
        
        .setup-header {
            text-align: center;
            background: rgba(255,255,255,0.98);
            backdrop-filter: blur(30px);
            padding: 4rem 3rem;
            border-radius: 32px;
            box-shadow: 
                0 20px 60px rgba(0,0,0,0.1),
                0 8px 25px rgba(0,0,0,0.05);
            border: 1px solid rgba(255,255,255,0.2);
            animation: slideInDown 0.8s ease-out;
            width: 100%;
            max-width: 100%;
            overflow-x: hidden;
            position: relative;
        }
        
        @keyframes slideInDown {
            from { opacity: 0; transform: translateY(-30px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        @keyframes slideInUp {
            from { opacity: 0; transform: translateY(30px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }
        
        .setup-logo {
            width: 140px;
            height: 140px;
            margin: 0 auto 2.5rem;
            border-radius: 24px;
            box-shadow: 0 10px 30px rgba(119, 88, 185, 0.3);
            transition: all 0.3s ease;
        }
        
        .setup-logo:hover {
            transform: translateY(-5px);
            box-shadow: 0 15px 40px rgba(119, 88, 185, 0.4);
        }
        
        .setup-title {
            font-size: 3rem;
            font-weight: 700;
            color: rgb(119, 88, 185);
            margin-bottom: 1.5rem;
            letter-spacing: -0.02em;
        }
        
        .setup-subtitle {
            font-size: 1.4rem;
            color: #666;
            margin-bottom: 3rem;
            font-weight: 300;
        }
        
        .welcome-message {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 2.5rem;
            border-radius: 20px;
            margin-bottom: 1.5rem;
            box-shadow: 0 10px 30px rgba(102, 126, 234, 0.3);
            position: relative;
            overflow: hidden;
        }
        
        .welcome-message::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: linear-gradient(45deg, rgba(255,255,255,0.1) 0%, transparent 100%);
            pointer-events: none;
        }
        
        .welcome-message h3 {
            font-size: 1.6rem;
            margin-bottom: 1rem;
            font-weight: 600;
        }
        
        .welcome-message p {
            font-size: 1.1rem;
            line-height: 1.6;
            opacity: 0.95;
        }
        
        .setup-form {
            background: rgba(255,255,255,0.98);
            backdrop-filter: blur(30px);
            border-radius: 32px;
            padding: 4rem;
            box-shadow: 
                0 20px 60px rgba(0,0,0,0.1),
                0 8px 25px rgba(0,0,0,0.05);
            border: 1px solid rgba(255,255,255,0.2);
            flex: 1;
            animation: slideInUp 0.8s ease-out 0.2s both;
        }
        
        .form-section {
            margin-bottom: 4rem;
            padding-bottom: 3rem;
            border-bottom: 2px solid rgba(102, 126, 234, 0.1);
            animation: fadeIn 0.6s ease-out;
        }
        
        .form-section:last-child {
            border-bottom: none;
            margin-bottom: 0;
            padding-bottom: 0;
        }
        
        .section-title {
            font-size: 1.8rem;
            font-weight: 700;
            color: #333;
            margin-bottom: 1.5rem;
            display: flex;
            align-items: center;
            gap: 1rem;
            letter-spacing: -0.01em;
        }
        
        .section-title .emoji {
            font-size: 2rem;
            filter: drop-shadow(0 2px 4px rgba(0,0,0,0.1));
        }
        
        .section-description {
            font-size: 1.1rem;
            color: #666;
            margin-bottom: 2.5rem;
            line-height: 1.7;
            max-width: 700px;
        }
        
        .toggle-group {
            display: flex;
            flex-direction: column;
            gap: 1.5rem;
        }
        
        .toggle-item {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 2rem;
            background: linear-gradient(135deg, #f8f9ff 0%, #e8edff 100%);
            border-radius: 20px;
            border: 2px solid transparent;
            border-left: 6px solid #667eea;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(102, 126, 234, 0.08);
        }
        
        .toggle-item:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(102, 126, 234, 0.15);
            border-color: rgba(102, 126, 234, 0.2);
        }
        
        .toggle-info {
            flex: 1;
            margin-right: 2rem;
        }
        
        .toggle-title {
            font-weight: 700;
            color: #333;
            margin-bottom: 0.5rem;
            font-size: 1.2rem;
        }
        
        .toggle-description {
            font-size: 1rem;
            color: #666;
            line-height: 1.5;
        }
        
        .toggle-switch {
            position: relative;
            width: 70px;
            height: 40px;
            flex-shrink: 0;
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
            background: linear-gradient(135deg, #ddd 0%, #ccc 100%);
            transition: 0.4s cubic-bezier(0.4, 0, 0.2, 1);
            border-radius: 40px;
            box-shadow: inset 0 2px 4px rgba(0,0,0,0.1);
        }
        
        .slider:before {
            position: absolute;
            content: "";
            height: 32px;
            width: 32px;
            left: 4px;
            bottom: 4px;
            background: white;
            transition: 0.4s cubic-bezier(0.4, 0, 0.2, 1);
            border-radius: 50%;
            box-shadow: 0 2px 8px rgba(0,0,0,0.2);
        }
        
        input:checked + .slider {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            box-shadow: 0 0 0 2px rgba(102, 126, 234, 0.2);
        }
        
        input:checked + .slider:before {
            transform: translateX(30px);
            box-shadow: 0 4px 12px rgba(0,0,0,0.3);
        }
        
        .form-group {
            margin-bottom: 2.5rem;
        }
        
        .form-label {
            display: block;
            font-weight: 700;
            color: #333;
            margin-bottom: 1rem;
            font-size: 1.2rem;
        }
        
        .form-input {
            width: 100%;
            max-width: 500px;
            padding: 1.5rem;
            font-size: 1.1rem;
            border: 2px solid #e1e5e9;
            border-radius: 16px;
            background: white;
            transition: all 0.3s ease;
            box-shadow: 0 2px 8px rgba(0,0,0,0.05);
        }
        
        .form-input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 
                0 0 0 4px rgba(102, 126, 234, 0.15),
                0 4px 20px rgba(102, 126, 234, 0.1);
            transform: translateY(-2px);
        }
        
        .requirements {
            background: linear-gradient(135deg, #fff8e1 0%, #fff3cd 100%);
            border: 2px solid #ffeaa7;
            border-radius: 16px;
            padding: 1.5rem;
            margin-top: 1rem;
            box-shadow: 0 2px 10px rgba(255, 193, 7, 0.1);
        }
        
        .requirements strong {
            color: #856404;
            font-weight: 700;
        }
        
        .requirements ul {
            margin: 0.5rem 0 0 1.5rem;
            color: #856404;
        }
        
        .requirements li {
            margin: 0.3rem 0;
        }
        
        .setup-actions {
            display: flex;
            gap: 2rem;
            justify-content: center;
            margin-top: 3rem;
            padding-top: 3rem;
            border-top: 2px solid rgba(102, 126, 234, 0.1);
        }
        
        .btn {
            padding: 1.5rem 3rem;
            font-size: 1.2rem;
            font-weight: 700;
            border: none;
            border-radius: 16px;
            cursor: pointer;
            transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
            text-decoration: none;
            display: inline-block;
            text-align: center;
            position: relative;
            overflow: hidden;
        }
        
        .btn::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255,255,255,0.3), transparent);
            transition: left 0.5s;
        }
        
        .btn:hover::before {
            left: 100%;
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            box-shadow: 0 8px 25px rgba(102, 126, 234, 0.3);
        }
        
        .btn-primary:hover {
            transform: translateY(-3px);
            box-shadow: 0 12px 35px rgba(102, 126, 234, 0.4);
        }
        
        .btn-primary:active {
            transform: translateY(-1px);
        }
        
        .hidden {
            display: none;
        }
        
        .notification {
            position: fixed;
            top: 2rem;
            right: 2rem;
            background: white;
            color: #333;
            padding: 1.5rem 2.5rem;
            border-radius: 16px;
            box-shadow: 0 15px 50px rgba(0,0,0,0.2);
            display: none;
            z-index: 1000;
            animation: slideInNotification 0.5s cubic-bezier(0.4, 0, 0.2, 1);
            border: 1px solid rgba(0,0,0,0.1);
        }
        
        @keyframes slideInNotification {
            from { 
                transform: translateX(100%) scale(0.9);
                opacity: 0;
            }
            to { 
                transform: translateX(0) scale(1);
                opacity: 1;
            }
        }
        
        .notification.success {
            border-left: 6px solid #28a745;
            background: linear-gradient(135deg, #f8fff9 0%, #e6ffed 100%);
        }
        
        .notification.error {
            border-left: 6px solid #dc3545;
            background: linear-gradient(135deg, #fff8f8 0%, #ffeded 100%);
        }
        
        /* Mobile responsive */
        @media (max-width: 768px) {
            body {
                padding: 0.5rem 0;
                overflow-x: hidden;
            }
            
            .setup-container {
                padding: 0 0.75rem;
                gap: 1.5rem;
                max-width: 100%;
                overflow-x: hidden;
            }
            
            .setup-header {
                padding: 2rem 1.5rem;
                border-radius: 20px;
                margin: 0;
                width: 100%;
                max-width: 100%;
                box-sizing: border-box;
            }
            
            .setup-title {
                font-size: 2rem;
                line-height: 1.2;
            }
            
            .setup-subtitle {
                font-size: 1rem;
            }
            
            .welcome-message {
                padding: 1.5rem;
                margin-bottom: 1rem;
                width: 100%;
                max-width: 100%;
                box-sizing: border-box;
            }
            
            .welcome-message h3 {
                font-size: 1.3rem;
            }
            
            .welcome-message p {
                font-size: 1rem;
            }
            
            .setup-form {
                padding: 2rem 1.5rem;
                border-radius: 20px;
                width: 100%;
                max-width: 100%;
                box-sizing: border-box;
                overflow-x: hidden;
            }
            
            .form-section {
                margin-bottom: 2.5rem;
                padding-bottom: 1.5rem;
                width: 100%;
                overflow-x: hidden;
            }
            
            .section-title {
                font-size: 1.4rem;
                line-height: 1.3;
                word-wrap: break-word;
            }
            
            .section-description {
                font-size: 1rem;
                margin-bottom: 2rem;
                word-wrap: break-word;
                max-width: 100%;
            }
            
            .toggle-item {
                padding: 1.25rem;
                flex-direction: column;
                align-items: flex-start;
                gap: 1rem;
                width: 100%;
                max-width: 100%;
                box-sizing: border-box;
            }
            
            .toggle-info {
                margin-right: 0;
                width: 100%;
                word-wrap: break-word;
            }
            
            .toggle-title {
                font-size: 1.1rem;
            }
            
            .toggle-description {
                font-size: 0.95rem;
            }
            
            .toggle-switch {
                align-self: flex-end;
                flex-shrink: 0;
            }
            
            .form-group {
                margin-bottom: 2rem;
                width: 100%;
            }
            
            .form-label {
                font-size: 1.1rem;
            }
            
            .form-input {
                width: 100%;
                max-width: 100%;
                padding: 1.25rem;
                box-sizing: border-box;
                font-size: 1rem;
            }
            
            .requirements {
                padding: 1.25rem;
                margin-top: 0.75rem;
                width: 100%;
                max-width: 100%;
                box-sizing: border-box;
                word-wrap: break-word;
            }
            
            .requirements ul {
                margin: 0.5rem 0 0 1rem;
            }
            
            .setup-actions {
                flex-direction: column;
                align-items: center;
                gap: 1rem;
                margin-top: 2rem;
                padding-top: 2rem;
                width: 100%;
            }
            
            .btn {
                width: 100%;
                max-width: 280px;
                padding: 1.25rem 2rem;
                font-size: 1.1rem;
                box-sizing: border-box;
            }
            
            .notification {
                top: 0.5rem;
                right: 0.5rem;
                left: 0.5rem;
                margin: 0;
                max-width: calc(100vw - 1rem);
                box-sizing: border-box;
            }
        }
        
        @media (max-width: 480px) {
            body {
                padding: 0.25rem 0;
            }
            
            .setup-container {
                padding: 0 0.5rem;
                gap: 1rem;
            }
            
            .setup-header {
                padding: 1.5rem 1rem;
                border-radius: 16px;
            }
            
            .setup-logo {
                width: 100px;
                height: 100px;
                margin-bottom: 1.5rem;
            }
            
            .setup-title {
                font-size: 1.75rem;
            }
            
            .setup-subtitle {
                font-size: 0.95rem;
            }
            
            .welcome-message {
                padding: 1.25rem;
                border-radius: 16px;
            }
            
            .welcome-message h3 {
                font-size: 1.2rem;
            }
            
            .setup-form {
                padding: 1.5rem 1rem;
                border-radius: 16px;
            }
            
            .form-section {
                margin-bottom: 2rem;
                padding-bottom: 1.25rem;
            }
            
            .section-title {
                font-size: 1.3rem;
                gap: 0.75rem;
            }
            
            .section-title .emoji {
                font-size: 1.5rem;
            }
            
            .toggle-item {
                padding: 1rem;
                border-radius: 16px;
            }
            
            .form-input {
                padding: 1rem;
                border-radius: 12px;
            }
            
            .requirements {
                padding: 1rem;
                border-radius: 12px;
            }
            
            .btn {
                padding: 1rem 1.5rem;
                border-radius: 12px;
                max-width: 260px;
            }
        }
        
        @media (max-width: 360px) {
            .setup-container {
                padding: 0 0.25rem;
            }
            
            .setup-header {
                padding: 1.25rem 0.75rem;
            }
            
            .setup-form {
                padding: 1.25rem 0.75rem;
            }
            
            .toggle-item {
                padding: 0.75rem;
            }
            
            .form-input {
                padding: 0.875rem;
            }
            
            .btn {
                max-width: 240px;
                padding: 0.875rem 1.25rem;
            }
        }
    </style>
    
    <script>
        // Configuration
        let setupData = {
            ghostKeyEnabled: true,
            ghostPowerEnabled: true,
            bluetoothEnabled: true,
            wifiPassword: '',
            webPassword: ''
        };
        
        function showNotification(message, type = 'success') {
            const notification = document.getElementById('notification');
            notification.textContent = message;
            notification.className = `notification ${type}`;
            notification.style.display = 'block';
            setTimeout(() => { 
                notification.style.display = 'none'; 
            }, 5000);
        }
        
        function updateBluetoothVisibility() {
            const ghostKeyEnabled = document.getElementById('ghostKeyEnabled').checked;
            const bluetoothSection = document.getElementById('bluetoothSection');
            
            if (ghostKeyEnabled) {
                bluetoothSection.classList.remove('hidden');
            } else {
                bluetoothSection.classList.add('hidden');
                document.getElementById('bluetoothEnabled').checked = false;
            }
        }
        
        function validateForm() {
            const ghostKey = document.getElementById('ghostKeyEnabled').checked;
            const ghostPower = document.getElementById('ghostPowerEnabled').checked;
            const wifiPassword = document.getElementById('wifiPassword').value;
            const webPassword = document.getElementById('webPassword').value;
            
            if (!ghostKey && !ghostPower) {
                showNotification('At least one system (Ghost Key or Ghost Power) must be enabled', 'error');
                return false;
            }
            
            if (wifiPassword.length < 8) {
                showNotification('WiFi password must be at least 8 characters', 'error');
                return false;
            }
            
            if (webPassword.length < 4) {
                showNotification('Web interface password must be at least 4 characters', 'error');
                return false;
            }
            
            return true;
        }
        
        async function completeSetup() {
            if (!validateForm()) {
                return;
            }
            
            const setupData = {
                ghostKeyEnabled: document.getElementById('ghostKeyEnabled').checked,
                ghostPowerEnabled: document.getElementById('ghostPowerEnabled').checked,
                bluetoothEnabled: document.getElementById('bluetoothEnabled').checked,
                wifiPassword: document.getElementById('wifiPassword').value,
                webPassword: document.getElementById('webPassword').value
            };
            
            try {
                const response = await fetch('/complete_setup', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(setupData)
                });
                
                if (response.ok) {
                    // Setup completed successfully - password is now stored on server
                    showNotification('Setup completed successfully! Redirecting to main interface...');
                    setTimeout(() => {
                        window.location.reload();
                    }, 2000);
                } else {
                    const errorText = await response.text();
                    showNotification('Setup failed: ' + errorText, 'error');
                }
            } catch (error) {
                showNotification('Setup failed: ' + error.message, 'error');
            }
        }
        
        // Initialize page
        document.addEventListener('DOMContentLoaded', () => {
            updateBluetoothVisibility();
            
            // Add event listeners
            document.getElementById('ghostKeyEnabled').addEventListener('change', updateBluetoothVisibility);
        });
    </script>
</head>
<body>
    <div class="setup-container">
        <!-- Header Section -->
        <div class="setup-header">
            <img src="/logo" class="setup-logo" alt="JDI Logo" onerror="this.style.display='none'">
            <h1 class="setup-title">Welcome to Ghost Key</h1>
            <p class="setup-subtitle">Secure Vehicle Access System</p>
            <div class="welcome-message">
                <h3>First Time Setup</h3>
                <p>Let's configure your Ghost Key system. This will only take a few minutes and you can change any of these settings later in the configuration interface.</p>
            </div>
        </div>
        
        <!-- Setup Form -->
        <div class="setup-form">
            <!-- System Configuration Section -->
            <div class="form-section">
                <h2 class="section-title">
                    System Configuration
                </h2>
                <p class="section-description">
                    Choose which systems to enable. Ghost Key provides RFID/Bluetooth authentication with push-to-start functionality. 
                    Ghost Power provides security relay control for vehicle immobilization.
                </p>
                
                <div class="toggle-group">
                    <div class="toggle-item">
                        <div class="toggle-info">
                            <div class="toggle-title">Ghost Key System</div>
                            <div class="toggle-description">Bluetooth authentication and Push-to-start control</div>
                        </div>
                        <label class="toggle-switch">
                            <input type="checkbox" id="ghostKeyEnabled" checked>
                            <span class="slider"></span>
                        </label>
                    </div>
                    
                    <div class="toggle-item">
                        <div class="toggle-info">
                            <div class="toggle-title">Ghost Power System</div>
                            <div class="toggle-description">Security relay control and Vehicle immobilization</div>
                        </div>
                        <label class="toggle-switch">
                            <input type="checkbox" id="ghostPowerEnabled" checked>
                            <span class="slider"></span>
                        </label>
                    </div>
                </div>
                
                <div class="requirements">
                    <strong>Important:</strong> At least one system must remain enabled.
                </div>
            </div>
            
            <!-- Bluetooth Configuration Section -->
            <div class="form-section" id="bluetoothSection">
                <h2 class="section-title">
                    Bluetooth Configuration
                </h2>
                <p class="section-description">
                    Enable Bluetooth for smartphone proximity authentication. When enabled, your phone will automatically authenticate you when you're nearby your vehicle.
                </p>
                
                <div class="toggle-group">
                    <div class="toggle-item">
                        <div class="toggle-info">
                            <div class="toggle-title">Bluetooth Authentication</div>
                            <div class="toggle-description">Automatic proximity detection using your smartphone</div>
                        </div>
                        <label class="toggle-switch">
                            <input type="checkbox" id="bluetoothEnabled" checked>
                            <span class="slider"></span>
                        </label>
                    </div>
                </div>
            </div>
            
            <!-- Security Configuration Section -->
            <div class="form-section">
                <h2 class="section-title">
                    Security Configuration
                </h2>
                <p class="section-description">
                    Set up your access passwords. The WiFi password protects access to the configuration network, 
                    and the web interface password protects the configuration interface.
                </p>
                
                <div class="form-group">
                    <label class="form-label" for="wifiPassword">WiFi Access Point Password</label>
                    <input type="password" id="wifiPassword" class="form-input" placeholder="Enter WiFi password" value="123456789">
                    <div class="requirements">
                        <strong>Requirements:</strong>
                        <ul>
                            <li>Minimum 8 characters</li>
                            <li>Used to connect to "Ghost Key Configuration" network</li>
                        </ul>
                    </div>
                </div>
                
                <div class="form-group">
                    <label class="form-label" for="webPassword">Web Interface Password</label>
                    <input type="password" id="webPassword" class="form-input" placeholder="Enter web interface password" value="1234">
                    <div class="requirements">
                        <strong>Requirements:</strong>
                        <ul>
                            <li>Minimum 4 characters</li>
                            <li>Used to access this configuration interface</li>
                        </ul>
                    </div>
                </div>
            </div>
            
            <!-- Setup Actions -->
            <div class="setup-actions">
                <button onclick="completeSetup()" class="btn btn-primary">Complete Setup & Continue</button>
            </div>
        </div>
    </div>
    
    <!-- Notification -->
    <div id="notification" class="notification"></div>
</body>
</html>
)rawliteral";

#endif // SETUP_HTML_H 