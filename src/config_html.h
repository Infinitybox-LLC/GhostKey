#ifndef CONFIG_HTML_H
#define CONFIG_HTML_H

const char config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no, shrink-to-fit=no, viewport-fit=cover">
    <title>Ghost Key Configuration</title>
    
    <style>
        /* ======================================== */
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
            flex-direction: column;
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
            body {
                overflow-x: hidden;
            }
            
            .main-container {
                flex-direction: column;
                overflow-x: hidden;
                width: 100%;
                max-width: 100%;
                position: relative;
            }
            
            /* Mobile Header - Collapsible */
            .app-header {
                background: rgba(255,255,255,0.98);
                backdrop-filter: blur(30px);
                padding: 0.5rem 1rem;
                overflow-x: hidden;
                width: 100%;
                max-width: 100%;
                box-sizing: border-box;
                transition: all 0.3s ease;
                position: sticky;
                top: 0;
                z-index: 100;
                border-bottom: 1px solid rgba(102, 126, 234, 0.1);
                height: 60px;
                display: flex;
                align-items: center;
            }
            
            /* Collapsible header states */
            .app-header.collapsed {
                padding: 0.25rem 1rem;
                height: 45px;
            }
            
            .app-header.minimal {
                padding: 0.25rem 1rem;
                height: 40px;
            }
            
            .app-logo {
                transition: all 0.3s ease;
            }
            
            .app-header.collapsed .app-logo {
                width: 32px;
                height: 32px;
            }
            
            .app-header.minimal .app-logo {
                width: 24px;
                height: 24px;
            }
            
            .app-name {
                transition: all 0.3s ease;
            }
            
            .app-header.collapsed .app-name {
                font-size: 1.2rem;
            }
            
            .app-header.minimal .app-name {
                font-size: 1rem;
            }
            
            /* Mobile Navigation Pills */
            .sidebar {
                width: 100%;
                margin: 0;
                position: sticky;
                top: 59px;
                display: flex;
                flex-direction: row !important;
                overflow-x: auto;
                overflow-y: hidden;
                padding: 0.5rem 1rem;
                max-width: 100vw;
                box-sizing: border-box;
                background: rgba(255,255,255,0.98);
                backdrop-filter: blur(30px);
                border-bottom: 1px solid rgba(102, 126, 234, 0.1);
                gap: 0.5rem;
                -webkit-overflow-scrolling: touch;
                scroll-snap-type: x mandatory;
                scrollbar-width: none;
                -ms-overflow-style: none;
                z-index: 200;
                border-radius: 0;
                box-shadow: 0 2px 10px rgba(0,0,0,0.1);
                margin-top: -1px;
            }
            
            .sidebar::-webkit-scrollbar {
                display: none;
            }
            
            /* Navigation Pills */
            .nav-item {
                min-width: 100px;
                max-width: 120px;
                display: flex;
                justify-content: center;
                align-items: center;
                padding: 0.75rem 1rem;
                margin: 0;
                border-radius: 20px;
                text-align: center;
                flex-shrink: 0;
                font-weight: 600;
                font-size: 0.85rem;
                background: rgba(102, 126, 234, 0.08);
                color: #667eea;
                border: 2px solid transparent;
                transition: all 0.3s ease;
                scroll-snap-align: start;
                white-space: nowrap;
                box-shadow: 0 2px 8px rgba(102, 126, 234, 0.1);
                height: 44px;
                cursor: pointer;
            }
            
            .nav-item:hover:not(.active) {
                background: rgba(102, 126, 234, 0.15);
                transform: translateY(-1px);
                box-shadow: 0 4px 12px rgba(102, 126, 234, 0.2);
            }
            
            .nav-item.active {
                background: linear-gradient(135deg, #667eea, #764ba2);
                color: white;
                transform: translateY(-2px);
                box-shadow: 0 6px 20px rgba(102, 126, 234, 0.4);
                border-color: rgba(255,255,255,0.2);
            }
            
            /* Exit button styling for mobile */
            .exit-nav {
                background: rgba(220, 53, 69, 0.1) !important;
                color: #dc3545 !important;
                border-color: rgba(220, 53, 69, 0.2) !important;
                min-width: 70px !important;
                max-width: 90px !important;
                font-size: 0.75rem !important;
                padding: 0.5rem 0.75rem !important;
            }
            
            .exit-nav:hover:not(.active) {
                background: rgba(220, 53, 69, 0.2) !important;
                color: #dc3545 !important;
            }
            
            .exit-nav.active {
                background: linear-gradient(135deg, #dc3545, #c82333) !important;
                color: white !important;
            }
            
            .content {
                padding: 1rem;
                width: 100%;
                max-width: 100%;
                overflow-x: hidden;
                box-sizing: border-box;
                margin-top: 0;
                position: relative;
                z-index: 10;
            }
            
            .card {
                padding: 1.5rem;
                width: 100%;
                max-width: 100%;
                overflow-x: hidden;
                box-sizing: border-box;
                margin-bottom: 1rem;
                position: relative;
                z-index: 5;
            }
            
            .status-grid {
                grid-template-columns: 1fr;
                gap: 0.75rem;
            }
            
            .device-actions {
                flex-direction: column;
                gap: 0.75rem;
            }
            
            .device-actions .btn {
                width: 100%;
                box-sizing: border-box;
            }
            
            /* Mobile button layouts */
            .card .btn {
                margin-bottom: 0.5rem;
            }
            
            .card .form-group {
                margin-bottom: 1.5rem;
            }
            
            /* Mobile confidence plotting buttons */
            .card h2 + div > div[style*="display: flex"] {
                flex-wrap: wrap;
                gap: 0.75rem;
            }
            
            .card h2 + div > div[style*="display: flex"] .btn {
                flex: 1;
                min-width: 120px;
            }
            
            .notification {
                top: 1rem;
                right: 1rem;
                left: 1rem;
                max-width: calc(100vw - 2rem);
                box-sizing: border-box;
            }
            
            .form-input {
                width: 100%;
                max-width: 100%;
                box-sizing: border-box;
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
                if (sectionName === 'config') {
                    // Only load the config-specific data (pulse time, auto lock)
                    loadSystemStatus().catch(error => {
                        console.log('Config section: some status elements may not be available');
                    });
                } else if (sectionName === 'bluetooth') {
                    loadBluetoothStatus();
                    updateDevices();
                    updateCalibrationStatus();
                    loadPlottingInterface(); // Load plotting interface when showing bluetooth
                } else if (sectionName === 'rfid') {
                    loadRfidKeys();
                    // Load RFID status and stored keys
                    loadSystemStatus().catch(error => {
                        console.log('RFID section: some status elements may not be available');
                    });
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
                showSection('config');
            }
        }

        function handleLogin(event) {
            event.preventDefault();
            const input = document.getElementById('passwordInput');
            const password = input.value;
            const errorDiv = document.getElementById('loginError');
            
            // Validate password against the server
            validatePassword(password).then(isValid => {
                if (isValid) {
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
            }).catch(error => {
                errorDiv.textContent = 'Error validating password';
                input.value = '';
            });
        }
        
        // Server-side password validation
        async function validatePassword(password) {
            try {
                const response = await fetch('/validate_web_password', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ password: password })
                });
                
                return response.ok;
            } catch (error) {
                console.error('Password validation error:', error);
                return false;
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
                if (!response.ok) {
                    throw new Error(`HTTP ${response.status}: ${response.statusText}`);
                }
                
                const data = await response.json();
                
                // Update RFID status (only if element exists - in RFID section)
                const rfidStatusElement = document.getElementById('rfidStatus');
                if (rfidStatusElement) {
                    rfidStatusElement.textContent = data.rfid ? 'Authenticated' : 'Not Authenticated';
                }
                
                // Update stored keys (only if element exists - in RFID section)
                const storedKeysElement = document.getElementById('storedKeys');
                if (storedKeysElement) {
                    storedKeysElement.textContent = data.keys || '0';
                }
                
                // Update form values - convert from milliseconds to seconds
                const pulseTimeElement = document.getElementById('pulseTime');
                if (pulseTimeElement && data.starterPulse) {
                    pulseTimeElement.value = (data.starterPulse / 1000).toFixed(1);
                }
                
                const autoLockTimeElement = document.getElementById('autoLockTime');
                if (autoLockTimeElement && data.autoLockTimeout) {
                    autoLockTimeElement.value = Math.round(data.autoLockTimeout / 1000);
                }
                
                // Load system configuration status
                await loadSystemConfig();
                
            } catch (error) {
                console.error('Error loading system status:', error);
                // Only show notification for actual network/server errors
                if (error.message && !error.message.includes('element')) {
                    showNotification('Error loading system status', 'error');
                }
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
            
            try {
                const response = await fetch('/update_web_password', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ password: newPassword })
                });
                
                if (!response.ok) throw new Error('Network response was not ok');
                
                document.getElementById('newPassword').value = '';
                document.getElementById('confirmPassword').value = '';
                showNotification('Web interface password updated successfully');
                
            } catch (error) {
                showNotification('Error updating web interface password: ' + error.message, 'error');
            }
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

        // Confidence Monitoring Functions
        async function loadConfidenceMonitoring() {
            try {
                const response = await fetchWithTimeout('/rssi_analysis');
                if (response.ok) {
                    const data = await response.json();
                    updateConfidenceUI(data);
                }
            } catch (error) {
                console.error('Error loading confidence data:', error);
            }
        }

        // Calibration Functions
        let calibrationInterval = null;

        async function startCalibration() {
            try {
                const response = await fetchWithTimeout('/calibration_start', { method: 'POST' });
                if (!response.ok) {
                    const errorText = await response.text();
                    throw new Error(errorText);
                }
                
                showNotification('Calibration started - keep your phone in position for 30 seconds');
                
                // Update UI
                document.getElementById('startCalibrationBtn').disabled = true;
                document.getElementById('stopCalibrationBtn').disabled = false;
                
                // Start status polling
                calibrationInterval = setInterval(updateCalibrationStatus, 1000);
                
            } catch (error) {
                showNotification('Error starting calibration: ' + error.message, 'error');
            }
        }

        async function stopCalibration() {
            try {
                const response = await fetchWithTimeout('/calibration_stop', { method: 'POST' });
                if (!response.ok) {
                    const errorText = await response.text();
                    throw new Error(errorText);
                }
                
                showNotification('Calibration stopped and offset calculated');
                stopCalibrationPolling();
                
            } catch (error) {
                showNotification('Error stopping calibration: ' + error.message, 'error');
            }
        }

        async function resetCalibration() {
            if (!confirm('Are you sure you want to reset calibration to default?')) {
                return;
            }
            
            try {
                const response = await fetchWithTimeout('/calibration_reset', { method: 'POST' });
                if (!response.ok) {
                    const errorText = await response.text();
                    throw new Error(errorText);
                }
                
                showNotification('Calibration reset to default');
                updateCalibrationStatus();
                
            } catch (error) {
                showNotification('Error resetting calibration: ' + error.message, 'error');
            }
        }

        async function updateCalibrationStatus() {
            try {
                const response = await fetchWithTimeout('/calibration_status');
                if (response.ok) {
                    const data = await response.json();
                    
                    // Update status display
                    document.getElementById('calibrationStatus').textContent = 
                        data.isCalibrating ? 'Active' : 'Inactive';
                    document.getElementById('calibrationOffset').textContent = 
                        data.offset.toFixed(1);
                    document.getElementById('calibrationSamples').textContent = 
                        data.sampleCount;
                    
                    if (data.isCalibrating) {
                        const timeRemaining = Math.ceil(data.timeRemaining / 1000);
                        document.getElementById('calibrationTimer').textContent = 
                            timeRemaining + 's';
                        
                        // Auto-stop when complete
                        if (data.timeRemaining <= 0) {
                            stopCalibrationPolling();
                            showNotification('Calibration completed automatically');
                        }
                    } else {
                        document.getElementById('calibrationTimer').textContent = '--';
                        stopCalibrationPolling();
                    }
                }
            } catch (error) {
                console.error('Error updating calibration status:', error);
            }
        }

        function stopCalibrationPolling() {
            if (calibrationInterval) {
                clearInterval(calibrationInterval);
                calibrationInterval = null;
            }
            
            // Reset UI
            document.getElementById('startCalibrationBtn').disabled = false;
            document.getElementById('stopCalibrationBtn').disabled = true;
        }

        // Update confidence monitoring UI
        function updateConfidenceUI(data) {
            document.getElementById('totalConfidence').textContent = data.confidence + '%';
            document.getElementById('stabilityScore').textContent = data.stabilityScore.toFixed(1) + '/35';
            document.getElementById('trendScore').textContent = data.trendScore.toFixed(1) + '/25';
            document.getElementById('strengthScore').textContent = data.strengthScore.toFixed(1) + '/40';
            document.getElementById('signalMean').textContent = data.mean.toFixed(1) + ' dBm';
            
            // Enhanced metrics
            document.getElementById('sampleQuality').textContent = (data.sampleQuality || 0).toFixed(1) + '%';
            document.getElementById('trendSlope').textContent = (data.averagedSlope || 0).toFixed(4);
            
            // Signal status
            let signalText = 'Active';
            if (data.signalLost) {
                signalText = '🔴 Signal Lost';
            } else if (data.sampleQuality < 50) {
                signalText = '🟡 Weak Signal';
            } else {
                signalText = '🟢 Strong Signal';
            }
            document.getElementById('signalStatus').textContent = signalText;

            // Movement status
            let movementText = 'Stable';
            if (data.isApproaching) {
                movementText = '🚶 Approaching';
            } else if (!data.isStable) {
                movementText = '📱 Moving';
            }
            document.getElementById('movementStatus').textContent = movementText;

            // Authentication status with color coding
            const authStatus = document.getElementById('authenticationStatus');
            const isAuthenticated = data.confidence >= data.authThreshold;
            
            if (isAuthenticated) {
                authStatus.innerHTML = '<span style="color: #28a745;">🟢 Authenticated</span>';
            } else {
                const percentage = (data.confidence / data.authThreshold * 100).toFixed(0);
                authStatus.innerHTML = `<span style="color: #dc3545;">⚫ Not Authenticated (${percentage}% of required)</span>`;
            }

            // Color-code confidence based on level
            const confidenceElement = document.getElementById('totalConfidence');
            if (data.confidence >= 65) {
                confidenceElement.style.color = '#28a745'; // Green - authenticated
            } else if (data.confidence >= 45) {
                confidenceElement.style.color = '#ffc107'; // Yellow - close to authentication
            } else {
                confidenceElement.style.color = '#dc3545'; // Red - not authenticated
            }
        }

        // Confidence Plotting Functions
        let plotData = [];
        let isPlotting = false;
        let plotInterval = null;
        let chart = null;

        async function loadPlottingInterface() {
            // Initialize chart if not already done
            if (!chart) {
                initializeChart();
            }
            
            // Update status
            updatePlotStatus();
        }

        function initializeChart() {
            const canvas = document.getElementById('confidenceChart');
            const ctx = canvas.getContext('2d');
            
            // Set canvas size
            canvas.width = canvas.offsetWidth;
            canvas.height = canvas.offsetHeight;
            
            // Initialize chart
            chart = {
                canvas: canvas,
                ctx: ctx,
                width: canvas.width,
                height: canvas.height,
                padding: 40
            };
            
            drawChart();
        }

        function drawChart() {
            if (!chart) return;
            
            const ctx = chart.ctx;
            const width = chart.width;
            const height = chart.height;
            const padding = chart.padding;
            
            // Clear canvas
            ctx.clearRect(0, 0, width, height);
            
            if (plotData.length === 0) {
                // Show overlay message
                document.getElementById('chartOverlay').style.display = 'block';
                return;
            }
            
            document.getElementById('chartOverlay').style.display = 'none';
            
            // Calculate scales
            const maxConfidence = Math.max(...plotData.map(d => d.confidence), 100);
            const minConfidence = Math.min(...plotData.map(d => d.confidence), 0);
            const timeRange = plotData.length > 1 ? 
                plotData[plotData.length - 1].timestamp - plotData[0].timestamp : 60000;
            
            // Draw grid
            ctx.strokeStyle = '#f0f0f0';
            ctx.lineWidth = 1;
            
            // Horizontal grid lines (confidence levels)
            for (let i = 0; i <= 10; i++) {
                const y = padding + (height - 2 * padding) * (1 - i / 10);
                ctx.beginPath();
                ctx.moveTo(padding, y);
                ctx.lineTo(width - padding, y);
                ctx.stroke();
                
                // Confidence labels
                ctx.fillStyle = '#999';
                ctx.font = '12px sans-serif';
                ctx.textAlign = 'right';
                ctx.fillText((i * 10) + '%', padding - 10, y + 4);
            }
            
            // Draw threshold lines
            ctx.strokeStyle = '#28a745';
            ctx.lineWidth = 2;
            ctx.setLineDash([5, 5]);
            const authY = padding + (height - 2 * padding) * (1 - 0.65);
            ctx.beginPath();
            ctx.moveTo(padding, authY);
            ctx.lineTo(width - padding, authY);
            ctx.stroke();
            
            ctx.strokeStyle = '#ffc107';
            const deauthY = padding + (height - 2 * padding) * (1 - 0.45);
            ctx.beginPath();
            ctx.moveTo(padding, deauthY);
            ctx.lineTo(width - padding, deauthY);
            ctx.stroke();
            ctx.setLineDash([]);
            
            // Draw data line
            if (plotData.length > 1) {
                ctx.strokeStyle = '#667eea';
                ctx.lineWidth = 3;
                ctx.beginPath();
                
                for (let i = 0; i < plotData.length; i++) {
                    const x = padding + (width - 2 * padding) * (i / (plotData.length - 1));
                    const y = padding + (height - 2 * padding) * (1 - plotData[i].confidence / 100);
                    
                    if (i === 0) {
                        ctx.moveTo(x, y);
                    } else {
                        ctx.lineTo(x, y);
                    }
                }
                ctx.stroke();
                
                // Draw data points
                ctx.fillStyle = '#667eea';
                for (let i = 0; i < plotData.length; i++) {
                    const x = padding + (width - 2 * padding) * (i / (plotData.length - 1));
                    const y = padding + (height - 2 * padding) * (1 - plotData[i].confidence / 100);
                    
                    ctx.beginPath();
                    ctx.arc(x, y, 3, 0, 2 * Math.PI);
                    ctx.fill();
                }
            }
        }

        function updatePlotStatus() {
            document.getElementById('plotStatus').textContent = isPlotting ? 'Recording' : 'Stopped';
            document.getElementById('plotDataPoints').textContent = plotData.length;
            
            if (plotData.length > 0) {
                const latest = plotData[plotData.length - 1];
                document.getElementById('plotCurrentConfidence').textContent = latest.confidence.toFixed(1) + '%';
            } else {
                document.getElementById('plotCurrentConfidence').textContent = '0%';
            }
        }

        async function startPlotting() {
            if (isPlotting) return;
            
            isPlotting = true;
            document.getElementById('startPlotBtn').disabled = true;
            document.getElementById('stopPlotBtn').disabled = false;
            
            // Clear old data
            plotData = [];
            updatePlotStatus();
            drawChart();
            
            // Start polling
            plotInterval = setInterval(async () => {
                try {
                    const response = await fetchWithTimeout('/rssi_analysis');
                    if (response.ok) {
                        const data = await response.json();
                        
                        // Add data point
                        plotData.push({
                            timestamp: Date.now(),
                            confidence: data.confidence
                        });
                        
                        // Keep only last 60 seconds of data
                        const cutoffTime = Date.now() - 60000;
                        plotData = plotData.filter(d => d.timestamp > cutoffTime);
                        
                        updatePlotStatus();
                        drawChart();
                    }
                } catch (error) {
                    console.error('Error fetching plot data:', error);
                }
            }, 1000); // Update every second
            
            showNotification('Started recording confidence data');
        }

        function stopPlotting() {
            if (!isPlotting) return;
            
            isPlotting = false;
            document.getElementById('startPlotBtn').disabled = false;
            document.getElementById('stopPlotBtn').disabled = true;
            
            if (plotInterval) {
                clearInterval(plotInterval);
                plotInterval = null;
            }
            
            updatePlotStatus();
            showNotification('Stopped recording confidence data');
        }

        function clearPlot() {
            plotData = [];
            updatePlotStatus();
            drawChart();
            showNotification('Cleared plot data');
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

        // Mobile header collapse functionality
        let lastScrollTop = 0;
        function handleHeaderCollapse() {
            if (window.innerWidth <= 768) {
                const header = document.querySelector('.app-header');
                const sidebar = document.querySelector('.sidebar');
                const scrollTop = window.pageYOffset || document.documentElement.scrollTop;
                
                if (scrollTop > 30 && scrollTop > lastScrollTop) {
                    // Scrolling down
                    if (scrollTop > 100) {
                        header.className = 'app-header minimal';
                        if (sidebar) sidebar.style.top = '39px';
                    } else {
                        header.className = 'app-header collapsed';
                        if (sidebar) sidebar.style.top = '44px';
                    }
                } else if (scrollTop < lastScrollTop - 20) {
                    // Scrolling up
                    if (scrollTop < 30) {
                        header.className = 'app-header';
                        if (sidebar) sidebar.style.top = '59px';
                    } else {
                        header.className = 'app-header collapsed';
                        if (sidebar) sidebar.style.top = '44px';
                    }
                }
                
                lastScrollTop = scrollTop;
            }
        }
        
        // Initialize page
        document.addEventListener('DOMContentLoaded', () => {
            checkAuth();
            
            // Add mobile header collapse listener
            window.addEventListener('scroll', handleHeaderCollapse);
            
            // Add click to expand header
            document.querySelector('.app-header')?.addEventListener('click', () => {
                if (window.innerWidth <= 768) {
                    const header = document.querySelector('.app-header');
                    const sidebar = document.querySelector('.sidebar');
                    header.className = 'app-header';
                    if (sidebar) sidebar.style.top = '59px';
                }
            });
            
            // Load system configuration on page load to set UI visibility
            setTimeout(() => {
                if (isAuthenticated) {
                    loadSystemConfig();
                }
            }, 1000);
            
            // Refresh data every 2 seconds for bluetooth (plotting), 15 seconds for others
            setInterval(() => {
                if (isAuthenticated) {
                    const activeSection = document.querySelector('.content-section.active');
                    if (activeSection && activeSection.id === 'bluetoothSection') {
                        loadBluetoothStatus();
                        updateDevices();
                        updateCalibrationStatus();
                        // Also update plotting if active
                        if (isPlotting) {
                            // Plotting updates are handled in the plotting interval
                        }
                    }
                }
            }, 2000);
            
            setInterval(() => {
                if (isAuthenticated) {
                    const activeSection = document.querySelector('.content-section.active');
                    if (activeSection && activeSection.id === 'configSection') {
                        loadSystemStatus();
                    } else if (activeSection && activeSection.id === 'rfidSection') {
                        loadRfidKeys();
                        loadSystemStatus(); // For RFID status and stored keys
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
                <div class="nav-item active" id="configNav" onclick="showSection('config')">
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
                    <span>Exit</span>
                </div>
            </nav>

            <!-- Content Area -->
            <main class="content">


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
                                When disabled, Bluetooth authentication will be completely turned off to save power and improve security. Changes take effect immediately.
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
                    
                    <div class="card">
                        <h2 class="card-title">🎯 Proximity Calibration</h2>
                        <div style="margin-bottom: 1.5rem;">
                            <p style="font-size: 0.9rem; color: #666; margin-bottom: 1rem;">
                                Calibrate the system for your specific installation. Position your phone where you want authentication to work, then run calibration.
                            </p>
                            <div style="display: flex; gap: 1rem; margin-bottom: 1rem; flex-wrap: wrap;">
                                <button onclick="startCalibration()" class="btn btn-primary" id="startCalibrationBtn">
                                    🎯 Start Calibration
                                </button>
                                <button onclick="stopCalibration()" class="btn btn-secondary" id="stopCalibrationBtn" disabled>
                                    ⏹️ Stop Early
                                </button>
                                <button onclick="resetCalibration()" class="btn btn-secondary">
                                    🔄 Reset to Default
                                </button>
                            </div>
                            <div style="background: #f8f9ff; padding: 1rem; border-radius: 12px;">
                                <div style="display: flex; justify-content: space-between; margin-bottom: 0.5rem;">
                                    <span>Calibration Status:</span>
                                    <span id="calibrationStatus">Inactive</span>
                                </div>
                                <div style="display: flex; justify-content: space-between; margin-bottom: 0.5rem;">
                                    <span>Current Offset:</span>
                                    <span id="calibrationOffset">0.0</span>
                                </div>
                                <div style="display: flex; justify-content: space-between; margin-bottom: 0.5rem;">
                                    <span>Time Remaining:</span>
                                    <span id="calibrationTimer">--</span>
                                </div>
                                <div style="display: flex; justify-content: space-between;">
                                    <span>Samples Collected:</span>
                                    <span id="calibrationSamples">0</span>
                                </div>
                            </div>
                            <p style="font-size: 0.8rem; color: #666; margin-top: 1rem;">
                                📍 <strong>Instructions:</strong><br>
                                1. Position your phone where you want authentication to work<br>
                                2. Click "Start Calibration" and keep your phone in position<br>
                                3. Wait 30 seconds for data collection to complete<br>
                                4. System will automatically calculate the optimal offset
                            </p>
                        </div>
                    </div>
                    
                    <div id="devicesList">
                        <div class="card">Loading devices...</div>
                    </div>
                    
                    <!-- Confidence Plotting (moved from separate section) -->
                    <div class="card">
                        <h2 class="card-title">📈 Confidence Over Time</h2>
                        <div style="margin-bottom: 1.5rem;">
                            <p style="font-size: 0.9rem; color: #666; margin-bottom: 1rem;">
                                Real-time plotting of confidence percentage. Click start to begin recording, stop to end. Data shows the last 60 seconds.
                            </p>
                            <div style="display: flex; gap: 1rem; margin-bottom: 1.5rem; flex-wrap: wrap;">
                                <button onclick="startPlotting()" class="btn btn-primary" id="startPlotBtn">
                                    ▶️ Start Recording
                                </button>
                                <button onclick="stopPlotting()" class="btn btn-secondary" id="stopPlotBtn" disabled>
                                    ⏹️ Stop Recording
                                </button>
                                <button onclick="clearPlot()" class="btn btn-secondary">
                                    🗑️ Clear Data
                                </button>
                            </div>
                            <div style="background: #f8f9ff; padding: 1rem; border-radius: 12px; margin-bottom: 1rem;">
                                <div style="display: flex; justify-content: space-between; margin-bottom: 0.5rem;">
                                    <span>Recording Status:</span>
                                    <span id="plotStatus">Stopped</span>
                                </div>
                                <div style="display: flex; justify-content: space-between; margin-bottom: 0.5rem;">
                                    <span>Data Points:</span>
                                    <span id="plotDataPoints">0</span>
                                </div>
                                <div style="display: flex; justify-content: space-between;">
                                    <span>Current Confidence:</span>
                                    <span id="plotCurrentConfidence">0%</span>
                                </div>
                            </div>
                        </div>
                        
                        <h3 class="card-title">Confidence Graph</h3>
                        <div style="background: #fff; border: 1px solid #e1e5e9; border-radius: 12px; padding: 1rem; height: 300px; position: relative;">
                            <canvas id="confidenceChart" width="800" height="280" style="width: 100%; height: 100%;"></canvas>
                            <div id="chartOverlay" style="position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); color: #999; font-size: 1.1rem;">
                                Click "Start Recording" to begin plotting
                            </div>
                        </div>
                        <div style="margin-top: 1rem; font-size: 0.9rem; color: #666;">
                            <div style="display: flex; gap: 2rem; justify-content: center; flex-wrap: wrap;">
                                <div style="display: flex; align-items: center; gap: 0.5rem;">
                                    <div style="width: 12px; height: 12px; background: #667eea; border-radius: 2px;"></div>
                                    <span>Confidence %</span>
                                </div>
                                <div style="display: flex; align-items: center; gap: 0.5rem;">
                                    <div style="width: 12px; height: 12px; background: #28a745; border-radius: 2px;"></div>
                                    <span>Auth Threshold (65%)</span>
                                </div>
                                <div style="display: flex; align-items: center; gap: 0.5rem;">
                                    <div style="width: 12px; height: 12px; background: #ffc107; border-radius: 2px;"></div>
                                    <span>Deauth Threshold (45%)</span>
                                </div>
                            </div>
                        </div>
                    </div>
                </section>



                <!-- Configuration Section -->
                <section class="content-section active" id="configSection">
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
                        
                        <!-- RFID Status -->
                        <div class="status-grid" style="margin-bottom: 2rem;">
                            <div class="status-item">
                                <div class="status-label">RFID Status</div>
                                <div class="status-value" id="rfidStatus">Loading...</div>
                            </div>
                            <div class="status-item">
                                <div class="status-label">Stored Keys</div>
                                <div class="status-value" id="storedKeys">Loading...</div>
                            </div>
                        </div>
                        
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