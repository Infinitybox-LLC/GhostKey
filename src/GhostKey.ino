// ========================================
// GHOSTKEY - ESP32 Car Security System
// ========================================
// Handles Bluetooth authentication, relay control, and web config
// Main features: secure car starting, proximity auth, pairing mode safety

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BleKeyboard.h>
#include <nvs_flash.h>
#include <esp_gap_ble_api.h>
#include <ArduinoJson.h>
#include "config_html.h"
#include "setup_html.h"

// ========================================
// JDI LOGO SVG - STORED IN PROGMEM
// ========================================
const char jdi_logo_svg[] PROGMEM = R"(<?xml version="1.0" encoding="UTF-8"?>
<svg id="Layer_1" data-name="Layer 1" xmlns="http://www.w3.org/2000/svg" version="1.1" viewBox="0 0 1999.44 1999.44">
  <defs>
    <style>
      .cls-1 {
        fill: #a800fc;
      }
      .cls-1, .cls-2 {
        stroke-width: 0px;
      }
      .cls-2 {
        fill: #000;
      }
    </style>
  </defs>
  <g>
    <path class="cls-2" d="M390.29,1560.64l6.88-16.14c.51-1.34,1.46-2.01,2.86-2.01h21.49c1.72,0,2.99-1.02,3.82-3.06l19.58-46.12c.64-1.34,1.59-2.01,2.86-2.01h18.81c.95,0,1.43.29,1.43.86,0,.26-.1.64-.29,1.15l-21.1,49.85c-1.66,3.88-3.28,7.1-4.87,9.65-1.59,2.55-3.36,4.54-5.3,5.97-1.94,1.43-4.2,2.43-6.78,3.01-2.58.57-5.68.86-9.31.86h-28.93c-.96,0-1.43-.29-1.43-.86,0-.32.1-.7.29-1.15Z"/>
    <path class="cls-2" d="M451.02,1553.39c0-2.35.57-4.87,1.72-7.54l8.4-19.96c1.21-2.93,2.48-5.38,3.82-7.35s2.85-3.56,4.54-4.78c1.69-1.21,3.61-2.09,5.78-2.62,2.16-.54,4.68-.81,7.54-.81h27.12c3.69,0,6.56.7,8.59,2.1,2.04,1.4,3.06,3.53,3.06,6.4,0,2.04-.57,4.39-1.72,7.07l-8.4,19.96c-1.27,3.05-2.67,5.65-4.2,7.78s-3.28,3.87-5.25,5.2c-1.97,1.34-4.2,2.31-6.68,2.91-2.48.61-5.32.91-8.5.91h-26.07c-3.31,0-5.76-.87-7.35-2.62-1.59-1.75-2.39-3.96-2.39-6.64ZM477.57,1543.93h10.69c1.46,0,2.55-.89,3.25-2.67l4.11-9.55c.25-.57.38-1.11.38-1.62,0-.7-.48-1.05-1.43-1.05h-10.7c-1.47,0-2.55.89-3.25,2.67l-4.11,9.55c-.26.57-.38,1.11-.38,1.62,0,.7.48,1.05,1.43,1.05Z"/>
    <path class="cls-2" d="M512.32,1560.83l13.46-31.8h-1.53c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h21.3c1.15,0,1.49.61,1.05,1.81l-1.15,2.58,8.21-3.53c1.27-.57,2.51-.86,3.72-.86h9.26c2.99,0,5.19.43,6.59,1.29,1.4.86,2.1,2.24,2.1,4.15,0,1.66-.61,3.95-1.81,6.88l-8.31,19.58c-.45,1.21-1.31,1.81-2.58,1.81h-17.38c-1.27,0-1.62-.6-1.05-1.81l4.49-10.51c.32-.76.48-1.3.48-1.62,0-.7-.51-1.05-1.53-1.05h-7.73l-13.46,31.8c-.45,1.21-1.31,1.81-2.58,1.81h-18.53c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M574.87,1554.91c0-2.23.6-4.74,1.81-7.54l9.17-21.68c1.27-2.99,2.56-5.46,3.87-7.4,1.3-1.94,2.78-3.52,4.44-4.73,1.65-1.21,3.55-2.05,5.68-2.53,2.13-.48,4.66-.72,7.59-.72h11.46c.64,0,1.18.06,1.62.19.44.13.92.32,1.43.57l5.73,3.34,10.22-23.97c.44-1.21,1.3-1.81,2.58-1.81h18.24c1.15,0,1.5.61,1.05,1.81l-22.73,53.48h1.62c1.27,0,1.62.61,1.05,1.82l-6.4,15.09c-.45,1.21-1.31,1.81-2.58,1.81h-10.03c-1.21,0-2.2-.29-2.96-.86l-6.88-5.25-11.27,5.25c-1.34.57-2.58.86-3.72.86h-10.7c-6.88,0-10.31-2.58-10.31-7.73ZM600.94,1542.88c0,.7.44,1.05,1.34,1.05h12.89l6.3-14.9h-12.89c-1.4,0-2.45.89-3.15,2.67l-4.11,9.55c-.26.57-.38,1.11-.38,1.62Z"/>
    <path class="cls-2" d="M643.72,1545.84l8.5-20.15c1.27-2.93,2.59-5.38,3.96-7.35,1.37-1.97,2.9-3.55,4.58-4.73,1.69-1.18,3.6-2.02,5.73-2.53,2.13-.51,4.57-.76,7.31-.76h11.46c1.27,0,2.29.25,3.06.76l5.73,3.34,1.05-2.29c.44-1.21,1.3-1.81,2.58-1.81h18.24c1.15,0,1.49.61,1.05,1.81l-13.56,31.8h1.62c1.27,0,1.62.61,1.05,1.82l-6.4,15.09c-.45,1.21-1.31,1.81-2.58,1.81h-10.03c-1.21,0-2.2-.29-2.96-.86l-6.88-5.25-11.27,5.25c-.7.32-1.34.54-1.91.67s-1.18.19-1.81.19h-10.7c-3.57,0-6.18-.62-7.83-1.86-1.66-1.24-2.48-3.07-2.48-5.49s.83-5.57,2.48-9.45ZM667.31,1542.88c0,.7.44,1.05,1.34,1.05h12.89l6.3-14.9h-12.89c-1.4,0-2.45.89-3.15,2.67l-4.11,9.55c-.26.57-.38,1.11-.38,1.62Z"/>
    <path class="cls-2" d="M705.89,1560.83l13.46-31.8h-2.1c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h21.68c1.15,0,1.49.61,1.05,1.81l-1.15,2.58,12.51-3.82c1.34-.38,2.61-.57,3.82-.57h4.2c4.77,0,8.24.7,10.41,2.1,2.16,1.4,3.25,3.63,3.25,6.68,0,2.8-.99,6.53-2.96,11.17l-12.89,30.37c-.57,1.34-1.53,2.01-2.87,2.01h-18.05c-1.34,0-1.72-.67-1.15-2.01l12.32-28.93c.25-.57.38-1.11.38-1.62,0-.7-.48-1.05-1.43-1.05h-12.99l-13.46,31.8c-.45,1.21-1.31,1.81-2.58,1.81h-18.43c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M808.45,1560.64l28.55-67.32c.64-1.34,1.59-2.01,2.86-2.01h49.37c8.34,0,12.51,3.15,12.51,9.46,0,2.93-.83,6.27-2.48,10.03l-13.56,31.99c-1.66,3.88-3.31,7.11-4.97,9.69-1.66,2.58-3.5,4.62-5.54,6.11-2.04,1.5-4.39,2.55-7.07,3.15-2.67.61-5.89.91-9.64.91h-48.89c-1.27,0-1.66-.67-1.15-2.01ZM858.77,1542.5c1.72,0,2.99-1.02,3.82-3.06l10.7-25.11c.32-.76.48-1.34.48-1.72,0-.89-.61-1.34-1.81-1.34h-18.72l-13.18,31.23h18.72Z"/>
    <path class="cls-2" d="M886.08,1560.83l13.46-31.8h-2.2c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h22.25c1.15,0,1.5.61,1.05,1.81l-20.72,48.7c-.45,1.21-1.31,1.81-2.58,1.81h-18.24c-1.15,0-1.5-.6-1.05-1.81ZM910.53,1503.16l5.44-12.7c.44-1.21,1.3-1.81,2.58-1.81h18.24c1.15,0,1.49.61,1.05,1.81l-5.44,12.7c-.45,1.21-1.31,1.81-2.58,1.81h-18.24c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M916.07,1560.83l6.4-14.99c.44-1.21,1.3-1.82,2.58-1.82h29.41c.83,0,1.46-.51,1.91-1.53.44-1.08.25-1.62-.57-1.62h-20.34c-5.6,0-8.4-2.07-8.4-6.21,0-1.72.54-3.92,1.62-6.59l2.2-5.25c1.08-2.61,2.13-4.73,3.15-6.35,1.02-1.62,2.18-2.9,3.49-3.82,1.3-.92,2.83-1.54,4.58-1.86,1.75-.32,3.9-.48,6.45-.48h39.63c1.15,0,1.5.61,1.05,1.81l-6.4,15.09c-.57,1.21-1.47,1.81-2.67,1.81h-26.83c-.57,0-1.07.3-1.48.91-.41.61-.62,1.16-.62,1.67,0,.38.25.57.76.57h19.86c6.05,0,9.07,2.07,9.07,6.21,0,1.91-.57,4.2-1.72,6.87l-2.39,5.54c-.96,2.29-1.96,4.2-3.01,5.73s-2.28,2.74-3.68,3.63c-1.4.89-3.01,1.53-4.82,1.91-1.81.38-3.96.57-6.45.57h-41.73c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M983.49,1555.39c0-1.46.27-3.1.81-4.92.54-1.82,1.26-3.84,2.15-6.07l6.49-15.37h-2.96c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h2.96l7.26-17.19c.57-1.21,1.43-1.82,2.58-1.82h18.72c1.27,0,1.62.61,1.05,1.82l-7.26,17.19h10.79c1.27,0,1.62.61,1.05,1.81l-6.49,15.09c-.57,1.21-1.43,1.81-2.58,1.81h-10.79l-5.16,12.22c-.32.76-.48,1.34-.48,1.72,0,.64.48.95,1.43.95h15.57c1.27,0,1.62.61,1.05,1.82l-6.4,15.09c-.45,1.21-1.31,1.81-2.58,1.81h-24.54c-3.88,0-6.64-.62-8.26-1.86-1.62-1.24-2.44-3.04-2.44-5.4Z"/>
    <path class="cls-2" d="M1027.51,1560.83l13.46-31.8h-1.53c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h21.29c1.15,0,1.5.61,1.05,1.81l-1.15,2.58,8.21-3.53c1.27-.57,2.51-.86,3.72-.86h9.26c2.99,0,5.19.43,6.59,1.29,1.4.86,2.1,2.24,2.1,4.15,0,1.66-.61,3.95-1.82,6.88l-8.31,19.58c-.45,1.21-1.31,1.81-2.58,1.81h-17.38c-1.27,0-1.62-.6-1.05-1.81l4.49-10.51c.32-.76.48-1.3.48-1.62,0-.7-.51-1.05-1.53-1.05h-7.74l-13.46,31.8c-.45,1.21-1.31,1.81-2.58,1.81h-18.53c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M1088.34,1560.83l13.46-31.8h-2.2c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h22.25c1.15,0,1.49.61,1.05,1.81l-20.72,48.7c-.45,1.21-1.31,1.81-2.58,1.81h-18.24c-1.15,0-1.5-.6-1.05-1.81ZM1112.78,1503.16l5.44-12.7c.44-1.21,1.3-1.81,2.58-1.81h18.24c1.15,0,1.5.61,1.05,1.81l-5.44,12.7c-.45,1.21-1.31,1.81-2.58,1.81h-18.24c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M1114.98,1560.83l6.4-15.09c.57-1.21,1.43-1.82,2.58-1.82h1.53l22.73-53.48c.44-1.21,1.3-1.81,2.58-1.81h18.24c1.15,0,1.5.61,1.05,1.81l-10.22,23.97,8.69-3.34c1.4-.51,2.67-.76,3.82-.76h11.36c6.88,0,10.31,2.58,10.31,7.73,0,2.23-.61,4.77-1.81,7.64l-9.17,21.68c-1.27,2.93-2.56,5.38-3.87,7.35-1.31,1.98-2.79,3.55-4.44,4.73-1.66,1.18-3.55,2.01-5.68,2.48-2.13.48-4.66.72-7.59.72h-10.79c-.64,0-1.18-.08-1.62-.24-.45-.16-.89-.4-1.34-.72l-6.59-5.16-1.72,4.11c-.51,1.34-1.47,2.01-2.86,2.01h-20.53c-1.15,0-1.5-.6-1.05-1.81ZM1147.35,1543.93h12.8c1.53,0,2.64-.89,3.34-2.67l4.11-9.55c.25-.57.38-1.08.38-1.53,0-.77-.51-1.15-1.53-1.15h-12.8l-6.3,14.9Z"/>
    <path class="cls-2" d="M1188.22,1555.2c0-2.48.76-5.54,2.29-9.17l14.42-33.9c.44-1.21,1.3-1.81,2.58-1.81h18.24c1.15,0,1.5.61,1.05,1.81l-12.41,29.13c-.26.57-.38,1.11-.38,1.62,0,.7.48,1.05,1.43,1.05h13.08l13.56-31.8c.44-1.21,1.3-1.81,2.58-1.81h18.24c1.15,0,1.5.61,1.05,1.81l-13.56,31.8h2.48c1.27,0,1.62.61,1.05,1.82l-6.4,15.09c-.45,1.21-1.31,1.81-2.58,1.81h-10.79c-1.21,0-2.2-.29-2.96-.86l-6.97-5.35-11.55,5.35c-1.34.57-2.58.86-3.72.86h-11.55c-6.11,0-9.17-2.48-9.17-7.45Z"/>
    <path class="cls-2" d="M1257.74,1555.39c0-1.46.27-3.1.81-4.92.54-1.82,1.26-3.84,2.15-6.07l6.49-15.37h-2.96c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h2.96l7.26-17.19c.57-1.21,1.43-1.82,2.58-1.82h18.72c1.27,0,1.62.61,1.05,1.82l-7.26,17.19h10.79c1.27,0,1.62.61,1.05,1.81l-6.49,15.09c-.57,1.21-1.43,1.81-2.58,1.81h-10.79l-5.16,12.22c-.32.76-.48,1.34-.48,1.72,0,.64.48.95,1.43.95h15.57c1.27,0,1.62.61,1.05,1.82l-6.4,15.09c-.45,1.21-1.31,1.81-2.58,1.81h-24.54c-3.88,0-6.64-.62-8.26-1.86-1.62-1.24-2.43-3.04-2.43-5.4Z"/>
    <path class="cls-2" d="M1303.77,1553.39c0-2.35.57-4.87,1.72-7.54l8.4-19.96c1.21-2.93,2.48-5.38,3.82-7.35,1.34-1.97,2.85-3.56,4.54-4.78,1.69-1.21,3.61-2.09,5.78-2.62,2.16-.54,4.68-.81,7.54-.81h27.12c3.69,0,6.56.7,8.59,2.1,2.04,1.4,3.06,3.53,3.06,6.4,0,2.04-.57,4.39-1.72,7.07l-8.4,19.96c-1.27,3.05-2.67,5.65-4.2,7.78s-3.28,3.87-5.25,5.2c-1.97,1.34-4.2,2.31-6.68,2.91-2.48.61-5.32.91-8.5.91h-26.07c-3.31,0-5.76-.87-7.35-2.62-1.59-1.75-2.39-3.96-2.39-6.64ZM1330.32,1543.93h10.7c1.46,0,2.54-.89,3.25-2.67l4.11-9.55c.25-.57.38-1.11.38-1.62,0-.7-.48-1.05-1.43-1.05h-10.69c-1.47,0-2.55.89-3.25,2.67l-4.11,9.55c-.25.57-.38,1.11-.38,1.62,0,.7.48,1.05,1.43,1.05Z"/>
    <path class="cls-2" d="M1365.08,1560.83l13.46-31.8h-1.53c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h21.29c1.15,0,1.5.61,1.05,1.81l-1.15,2.58,8.21-3.53c1.27-.57,2.51-.86,3.72-.86h9.26c2.99,0,5.19.43,6.59,1.29,1.4.86,2.1,2.24,2.1,4.15,0,1.66-.61,3.95-1.82,6.88l-8.31,19.58c-.45,1.21-1.31,1.81-2.58,1.81h-17.38c-1.27,0-1.62-.6-1.05-1.81l4.49-10.51c.32-.76.48-1.3.48-1.62,0-.7-.51-1.05-1.53-1.05h-7.74l-13.46,31.8c-.45,1.21-1.31,1.81-2.58,1.81h-18.53c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M1424.28,1560.83l6.4-14.99c.44-1.21,1.3-1.82,2.58-1.82h29.41c.83,0,1.46-.51,1.91-1.53.44-1.08.25-1.62-.57-1.62h-20.34c-5.6,0-8.4-2.07-8.4-6.21,0-1.72.54-3.92,1.62-6.59l2.2-5.25c1.08-2.61,2.13-4.73,3.15-6.35,1.02-1.62,2.18-2.9,3.49-3.82,1.3-.92,2.83-1.54,4.58-1.86,1.75-.32,3.9-.48,6.45-.48h39.63c1.15,0,1.5.61,1.05,1.81l-6.4,15.09c-.57,1.21-1.47,1.81-2.67,1.81h-26.83c-.57,0-1.07.3-1.48.91-.41.61-.62,1.16-.62,1.67,0,.38.25.57.76.57h19.86c6.05,0,9.07,2.07,9.07,6.21,0,1.91-.57,4.2-1.72,6.87l-2.39,5.54c-.96,2.29-1.96,4.2-3.01,5.73s-2.28,2.74-3.68,3.63c-1.4.89-3.01,1.53-4.82,1.91-1.81.38-3.96.57-6.45.57h-41.73c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M1523.12,1560.64l28.55-67.32c.64-1.34,1.59-2.01,2.86-2.01h19.86c.95,0,1.43.29,1.43.86,0,.26-.09.64-.29,1.15l-28.55,67.32c-.51,1.34-1.47,2.01-2.86,2.01h-19.86c-1.27,0-1.66-.67-1.15-2.01Z"/>
    <path class="cls-2" d="M1555.59,1560.83l13.46-31.8h-2.1c-1.15,0-1.5-.6-1.05-1.81l6.49-15.09c.44-1.21,1.3-1.81,2.58-1.81h21.68c1.15,0,1.5.61,1.05,1.81l-1.15,2.58,12.51-3.82c1.34-.38,2.61-.57,3.82-.57h4.2c4.77,0,8.24.7,10.41,2.1,2.16,1.4,3.25,3.63,3.25,6.68,0,2.8-.99,6.53-2.96,11.17l-12.89,30.37c-.57,1.34-1.53,2.01-2.86,2.01h-18.05c-1.34,0-1.72-.67-1.15-2.01l12.32-28.93c.25-.57.38-1.11.38-1.62,0-.7-.48-1.05-1.43-1.05h-12.99l-13.46,31.8c-.45,1.21-1.31,1.81-2.58,1.81h-18.43c-1.15,0-1.5-.6-1.05-1.81Z"/>
    <path class="cls-2" d="M1627.4,1547.27l8.98-21.1c1.34-3.05,2.69-5.6,4.06-7.64,1.37-2.04,2.94-3.66,4.73-4.87,1.78-1.21,3.79-2.07,6.02-2.58,2.23-.51,4.87-.76,7.93-.76h31.51c1.15,0,1.5.61,1.05,1.81l-6.4,15.09c-.45,1.21-1.31,1.81-2.58,1.81h-23.11c-1.4,0-2.45.89-3.15,2.67l-4.11,9.55c-.32.76-.48,1.34-.48,1.72,0,.64.48.95,1.43.95h23.11c1.27,0,1.62.61,1.05,1.82l-6.4,15.09c-.45,1.21-1.31,1.81-2.58,1.81h-32.09c-3.5,0-6.16-.7-7.97-2.1-1.82-1.4-2.72-3.44-2.72-6.11,0-2.1.57-4.49,1.72-7.16Z"/>
    <path class="cls-2" d="M1681.64,1560.64l6.97-16.52c.64-1.34,1.59-2.01,2.86-2.01h18.14c.96,0,1.43.29,1.43.86,0,.25-.09.64-.29,1.15l-6.97,16.52c-.51,1.34-1.46,2.01-2.86,2.01h-18.14c-1.27,0-1.66-.67-1.15-2.01Z"/>
  </g>
  <path class="cls-2" d="M452.96,1404.82l55.7-130.75c4.12-10.83,11.86-16.25,23.21-16.25h174.08c13.93,0,24.24-8.24,30.95-24.76l158.6-373.68c5.15-10.83,12.89-16.25,23.21-16.25h152.41c7.74,0,11.61,2.32,11.61,6.96,0,2.07-.77,5.16-2.32,9.28l-170.98,403.86c-13.42,31.47-26.57,57.52-39.46,78.14-12.9,20.64-27.21,36.75-42.94,48.35-15.74,11.61-34.04,19.73-54.93,24.37-20.89,4.64-46.03,6.96-75.43,6.96h-234.42c-7.74,0-11.6-2.32-11.6-6.96,0-2.57.77-5.67,2.32-9.28Z"/>
  <path class="cls-2" d="M911.74,1404.82l231.33-545.44c5.15-10.83,12.89-16.25,23.21-16.25h399.99c67.56,0,101.35,25.53,101.35,76.59,0,23.73-6.71,50.81-20.11,81.24l-109.86,259.18c-13.42,31.47-26.83,57.64-40.23,78.53-13.42,20.89-28.37,37.4-44.87,49.51-16.51,12.13-35.59,20.64-57.25,25.53-21.66,4.91-47.72,7.35-78.14,7.35h-396.12c-10.32,0-13.42-5.42-9.28-16.25ZM1319.47,1257.83c13.93,0,24.24-8.24,30.95-24.76l86.65-203.48c2.57-6.19,3.87-10.83,3.87-13.93,0-7.22-4.91-10.83-14.7-10.83h-151.64l-106.77,252.99h151.64Z"/>
  <path class="cls-2" d="M1539.97,1406.37l109.09-257.63h-17.79c-9.28,0-12.13-4.9-8.51-14.7l52.61-122.24c3.6-9.79,10.56-14.7,20.89-14.7h180.27c9.28,0,12.11,4.91,8.51,14.7l-167.89,394.57c-3.61,9.8-10.58,14.7-20.89,14.7h-147.77c-9.28,0-12.13-4.9-8.51-14.7Z"/>
  <path class="cls-1" d="M1734.92,927.73l52.05-121.45c4.72-12.82,13.83-19.24,27.35-19.24h137.46c20.65,0,22.51,16.97,17.79,29.79l-52.34,124.18c-4.73,12.84-13.85,19.25-27.35,19.25h-128.25c-12.15,0-37.27-4.84-26.71-32.52Z"/>
  <path class="cls-2" d="M763.11,619.09c-12.93-71.5-26.37-72.01-40.06-72.01s-35.5,15.97-58.06,34.23c-22.56,18.25-84.43,72.26-107.75,94.57,0,0,4.06-53.5,7.86-94.06,3.8-40.57-1.01-66.93-23.33-99.13-22.31-32.2-61.61-45.89-95.84-45.89-60.75,0-98.29,45.41-110.82,80.02-12.53,34.61-44.46,118.46-55.2,156.06,0,0-69.34-44.74-125.25-82.53-55.91-37.8-68.12-50-81.11-50-46.46,0-45.83,252.13-45.83,279.31s4.12,36.52,12.63,36.52,15.93-11.26,42.28-67.55c26.36-56.29,32.13-70.57,37.89-70.57s2.2,26.08,2.2,49.7,3.84,35.42,18.67,35.42,57.43-13.32,85.92-13.32,36.04,22.21,36.04,30.17c0,36.04-29.75,89.68-53.64,145.84-23.89,56.16-41.82,98.46-41.82,139.04s26.4,46.19,50.15,46.19,22.77-22.44,22.77-37.61,0-25.41,3.96-16.5c3.96,8.91,10.45,45.48,58.77,127.85,48.32,82.37,125.17,116.43,161.52,134.83,36.35,18.41,81.45,26.69,29.91-11.5-56.6-38.66-107.68-147.26-115.51-189.14-7.82-41.88-2.76-47.86.46-34.51,25.31,99.86,93.2,163.07,118.43,179.9,25.24,16.83,51.98-19.83,34.25-39.66-17.73-19.83-47.77-86.53-47.77-150.53,0-25.24,3.31-32.75,5.71-27.34,2.4,5.41,18.11,56.87,42.97,103.06,3-20.13,31.25-35.46,60.09-42.97,28.84-7.51,60.09-11.12,60.09-38.16s-40.56-88.04-60.09-120.19c-19.53-32.15-47.17-91.04-47.17-114.18,0-57.39,51.68-65.8,79.62-66.7,27.94-.9,48.98-1.5,58.89-17.43,9.92-15.92,13.22-67.61,13.82-86.83.6-19.23,5.41-8.41,18.63,33.65,13.22,42.07,25.67,73.89,35.31,89.86,9.63,15.97,15.47-.25,19.27-49.95,3.8-49.69,4.06-86.46-8.87-157.95ZM443.37,612.32c-6.03,4.57-18.95,12.69-34.51,12.69-29.02,0-47.41-19.73-47.63-63.73-.01-2.44,2.84-3.78,4.71-2.21,31.41,26.35,65.3,43.02,76.92,48.36,1.97.91,2.25,3.58.52,4.89ZM485.22,625.01c-15.56,0-28.48-8.12-34.51-12.69-1.73-1.31-1.46-3.98.52-4.89,11.62-5.34,45.5-22.02,76.92-48.36,1.87-1.57,4.72-.23,4.71,2.21-.22,44-18.62,63.73-47.63,63.73Z"/>
</svg>)";

// ========================================
// PWA MANIFEST JSON - STORED IN PROGMEM
// ========================================
const char manifest_json[] PROGMEM = R"({
  "name": "Ghost Key Configuration",
  "short_name": "Ghost Key",
  "id": "http://ghost-key.local/",
  "description": "Secure Vehicle Access System Configuration",
  "start_url": "http://ghost-key.local/",
  "display": "standalone",
  "background_color": "#764ba2",
  "theme_color": "#667eea",
  "orientation": "portrait-primary",
  "scope": "http://ghost-key.local/",
  "icons": [
    {
      "src": "/icon.png",
      "sizes": "32x32",
      "type": "image/png",
      "purpose": "any"
    },
    {
      "src": "/icon.png",
      "sizes": "192x192",
      "type": "image/png",
      "purpose": "any maskable"
    },
    {
      "src": "/icon.png",
      "sizes": "512x512",
      "type": "image/png",
      "purpose": "any maskable"
    }
  ]
})";

// ========================================
// DEBUG SYSTEM - DEFINED EARLY FOR ALL FUNCTIONS
// ========================================
#define DEBUG_SYSTEM 1
#define DEBUG_BUTTON 1
#define DEBUG_RELAY 1

#define DEBUG_PRINT(x) if(DEBUG_SYSTEM) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DEBUG_SYSTEM) Serial.println(x)
#define DEBUG_BUTTON_PRINT(x) if(DEBUG_BUTTON) Serial.print(x)
#define DEBUG_BUTTON_PRINTLN(x) if(DEBUG_BUTTON) Serial.println(x)
#define DEBUG_RELAY_PRINT(x) if(DEBUG_RELAY) Serial.print(x)
#define DEBUG_RELAY_PRINTLN(x) if(DEBUG_RELAY) Serial.println(x)

// ========================================
// DEBUG CONFIGURATION
// ========================================
#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_BASIC 1
#define DEBUG_LEVEL_VERBOSE 2
#define DEBUG_LEVEL_FULL 3
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_BASIC

// ========================================
// PIN DEFINITIONS
// ========================================
// Input pins (all active low with pullups)
#define BUTTON_PIN 32 // Changed from 34 - supports internal pullup
#define BRAKE_PIN 18 // Good
#define ACCESSORY_INPUT_PIN 19 // Ghost Power accessory input (external pullup)
#define BEEPER_PIN 25  // GPIO 25 will be used for the beeper

// Output pins
#define LED_PIN 23 // change
#define BUTTON_LED_PIN 26 // good

// RFID pins
#define RFID_DEMOD_OUT 12 // good
#define RFID_SHD 13 // good
#define RFID_MOD 27 // good
#define RFID_RDY_CLK 14 // good

// Relay control pins
#define RELAY_ACCESSORY 15 // good
#define RELAY_IGNITION1 5 // 
#define RELAY_IGNITION2 4 // good
#define RELAY_START 16 // good
#define RELAY_SECURITY 17 // Change all // good


// ========================================
// TIMING CONSTANTS
// ========================================
#define CONFIG_MODE_TIMEOUT 30000
#define AUTO_LOCK_TIMEOUT 30000
#define STARTER_PULSE_TIME 1500
#define DEBOUNCE_DELAY 50
#define LONG_PRESS_TIME 30000
#define BUTTON_LED_BLINK_RATE 500
#define MAX_STORED_KEYS 10
#define PAIRING_MODE_TIMEOUT 30000

// ========================================
// RFID CONSTANTS
// ========================================
#define RFID_DELAYVAL 384    // Standard delay for Manchester decode
#define RFID_TIMEOUT 2000    // Standard timeout for Manchester decode
#define MAX_RFID_KEYS 5      // Maximum number of RFID keys to store
#define RFID_AUTH_TIMEOUT 30000  // 30 seconds RFID authentication timeout

// ========================================
// BLUETOOTH CONFIGURATION
// ========================================
#define MAX_BONDS 3
#define BOND_STORAGE_NAMESPACE "ble_bonds"
#define DEVICE_NAME_KEY "dev_name_"
#define DEVICE_PRIORITY_KEY "dev_priority_"
#define MAX_RSSI -30
#define MIN_RSSI -100
#define RSSI_UPDATE_INTERVAL 1000
#define CONNECTION_TIMEOUT 30000
#define MIN_CONN_INTERVAL 0x10
#define MAX_CONN_INTERVAL 0x20
#define SLAVE_LATENCY 0
#define SUPERVISION_TIMEOUT 0x100

// RSSI validation constants
#define RSSI_MIN_VALID -100      // Minimum valid RSSI value for BLE
#define RSSI_MAX_VALID -10       // Maximum valid RSSI value for BLE
#define RSSI_INVALID_VALUE -99   // Value to use for invalid RSSI

// Statistical RSSI Analysis constants (CONFIDENCE-BASED AUTHENTICATION)
#define RSSI_SHORT_TERM_SIZE 10       // 1 second of readings (100ms intervals)
#define RSSI_MEDIUM_TERM_SIZE 50      // 5 seconds of readings  
#define RSSI_LONG_TERM_SIZE 300       // 30 seconds of readings
#define CONFIDENCE_AUTH_THRESHOLD 65.0f    // Minimum confidence % for authentication (easier to authenticate)
#define CONFIDENCE_DEAUTH_THRESHOLD 50.0f  // Confidence % to lose authentication (faster to deauthenticate)
#define STABILITY_WEIGHT 35.0f        // Max points for stability (35%)
#define TREND_WEIGHT 25.0f           // Max points for trend analysis (25%)
#define STRENGTH_WEIGHT 40.0f        // Max points for signal strength (40%)
#define MIN_READINGS_FOR_ANALYSIS 3  // Minimum readings before analysis (faster startup)
#define TREND_WINDOW_MS 8000         // 8 seconds for trend analysis (increased from 3s)
#define STABILITY_THRESHOLD 12.0f    // Max std deviation for "stable" (increased tolerance)
#define STRONG_SIGNAL_THRESHOLD -60  // RSSI above this = strong signal
#define WEAK_SIGNAL_THRESHOLD -80    // RSSI below this = weak signal
#define VERY_STRONG_SIGNAL_THRESHOLD -50  // RSSI above this = very strong signal
#define PROXIMITY_BONUS_THRESHOLD -55     // RSSI above this gets proximity bonus

// Enhanced confidence calculation constants
#define STATIONARY_BONUS_POINTS 10.0f     // Bonus for stationary strong signal
#define CONFIDENCE_MOMENTUM_RATE 0.25f    // 25% change rate for faster response
#define CONFIDENCE_MOMENTUM_RATE_FAST 0.40f    // Fast momentum for rapid changes
#define CONFIDENCE_CHANGE_THRESHOLD 20.0f      // Threshold for switching to fast momentum
#define STATIONARY_MIN_SAMPLES 8          // Minimum samples for stationary detection
#define STATIONARY_MAX_STDDEV 6.0f        // Max std dev for "stationary" signal

// Outlier rejection and signal quality constants
#define OUTLIER_REJECTION_STDDEV_MULT 2.5f    // Multiplier for outlier rejection
#define MIN_SAMPLE_QUALITY_WEIGHT 0.3f        // Minimum weight for sample quality
#define OPTIMAL_SAMPLE_COUNT 10               // Optimal number of samples for full weight
#define MEDIAN_FILTER_SIZE 5                  // Size of median filter for outlier rejection

// No-signal handling constants
#define NO_SIGNAL_DECAY_RATE 0.9f            // Decay rate when no signal (per second)
#define NO_SIGNAL_TIMEOUT_MS 3000            // Time before considering signal lost
#define SIGNAL_LOSS_FAST_DECAY_RATE 0.7f     // Faster decay for signal loss

// Sigmoid function constants for smooth signal strength scoring
#define SIGMOID_MIDPOINT -60.0f              // RSSI midpoint for sigmoid function
#define SIGMOID_STEEPNESS 0.3f               // Steepness of sigmoid curve

// Slope averaging constants for trend analysis
#define SLOPE_HISTORY_SIZE 3                 // Number of slope measurements to average
#define SLOPE_VARIANCE_THRESHOLD 0.001f      // Threshold for slope stability

// ========================================
// CURRENT MONITORING (Future Use)
// ========================================
#define CURRENT_SENSE_PIN 36
#define SHUNT_RESISTOR 0.1
#define ADC_VREF 3.3
#define ADC_RESOLUTION 4095
#define CURRENT_SCALE 10

// ========================================
// DATA STRUCTURES
// ========================================

// System states for the car ignition sequence
enum SystemState {
    OFF,
    ACCESSORY,
    IGNITION,
    RUNNING,
    CONFIG_MODE
};

// Button state tracking with debouncing
struct ButtonState {
    bool currentState;
    bool lastState;
    bool isPressed;
    bool isLongPress;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
};

// Data structures for system state management

// RSSI Calibration data structure - REMOVED (using confidence-based auth only)

// Statistical RSSI Analysis structures
struct RSSIStatistics {
    float mean = 0.0f;
    float standardDeviation = 0.0f;
    float variance = 0.0f;
    int8_t minimum = 0;
    int8_t maximum = -100;
    int validSamples = 0;
};

struct RSSIReading {
    int8_t rssi = -99;
    unsigned long timestamp = 0;
    bool valid = false;
};

struct RSSIAnalysis {
    // Circular buffers for different time windows
    RSSIReading shortTerm[RSSI_SHORT_TERM_SIZE];
    RSSIReading mediumTerm[RSSI_MEDIUM_TERM_SIZE];
    RSSIReading longTerm[RSSI_LONG_TERM_SIZE];
    
    // Buffer indices (circular)
    int shortTermIndex = 0;
    int mediumTermIndex = 0;
    int longTermIndex = 0;
    
    // Statistical analysis results
    RSSIStatistics shortTermStats;
    RSSIStatistics mediumTermStats;
    RSSIStatistics longTermStats;
    
    // Confidence scoring components
    float stabilityScore = 0.0f;      // 0-35 points
    float trendScore = 0.0f;          // 0-25 points  
    float strengthScore = 0.0f;       // 0-40 points
    float totalConfidence = 0.0f;     // 0-100 points
    
    // Trend analysis
    float trendDirection = 0.0f;      // Positive = improving, negative = degrading
    bool isApproaching = false;       // Detected approach pattern
    bool isStable = false;            // Signal is stable (parked)
    
    // Enhanced trend analysis with slope averaging
    float slopeHistory[SLOPE_HISTORY_SIZE] = {0};
    int slopeHistoryIndex = 0;
    float averagedSlope = 0.0f;
    
    // Signal quality and outlier rejection
    unsigned long lastSignalTime = 0;   // Time of last valid signal
    bool signalLost = false;            // True if signal has been lost
    float sampleQualityWeight = 1.0f;   // Weight based on sample quality
    
    // Adaptive calibration tracking
    float historicalSuccessRate = 0.0f;  // Track authentication success rate
    int authAttempts = 0;               // Total authentication attempts
    int authSuccesses = 0;              // Successful authentications
    bool needsCalibrationAdjustment = false;  // Flag for auto-calibration
    
    // Timing
    unsigned long lastUpdate = 0;
    unsigned long lastAnalysis = 0;
    
    // Device tracking
    esp_bd_addr_t deviceAddress = {0};
    bool hasDevice = false;
};

// ========================================
// GLOBAL VARIABLES
// ========================================

SystemState currentState = OFF;
unsigned long lastActivityTime = 0;
bool isConfigured = false;
bool isBluetoothPaired = false;
bool firstSetupComplete = false; // Track if initial setup is done
WebServer server(80);
Preferences preferences;

// Bluetooth authentication state
bool bluetoothAuthenticated = false;
bool isBleConnected = false;
esp_bd_addr_t connectedDeviceAddr = {0};
bool hasConnectedDevice = false;
bool isPairingMode = false;
unsigned long pairingModeStartTime = 0;
bool bluetoothEnabled = true; // Default to enabled
bool bluetoothInitialized = false;

// System modularity - Ghost Key vs Ghost Power
bool ghostKeyEnabled = true;    // RFID/Bluetooth/Push-to-start functionality
bool ghostPowerEnabled = true;  // Security relay functionality
bool accessoryInputAuth = false; // Authentication via brake + accessory input

// Bluetooth timing measurements
unsigned long systemBootTime = 0;
unsigned long bluetoothInitStartTime = 0;
unsigned long bluetoothInitCompleteTime = 0;
unsigned long firstRSSIScanStartTime = 0;
unsigned long firstRSSIScanCompleteTime = 0;
bool firstRSSIScanDone = false;

// RFID timing measurements
unsigned long rfidInitStartTime = 0;
unsigned long rfidInitCompleteTime = 0;
unsigned long firstRfidScanStartTime = 0;
unsigned long firstRfidReadCompleteTime = 0;
bool firstRfidReadDone = false;

// BLE objects
BleKeyboard bleKeyboard("Ghost Key", "Jordan Distributors, Inc", 100);
BLEServer* pServer = nullptr;
class MyServerCallbacks;

// Button state tracking
ButtonState buttonState = {false, false, false, false, 0, 0};
ButtonState brakeState = {false, false, false, false, 0, 0};

// RFID state variables
byte tagData[5];  // Holds the ID numbers from the tag
byte storedRfidKeys[MAX_RFID_KEYS][5];  // Array to store RFID keys
int numStoredKeys = 0;  // Number of keys currently stored
bool rfidPairingMode = false;  // True when waiting for RFID key to pair
bool rfidAuthenticated = false;  // True when valid RFID tag detected
unsigned long rfidAuthStartTime = 0;  // Time when RFID auth started

// ========================================
// BLUETOOTH CACHING SYSTEM
// ========================================

// Device caching structures for efficiency
struct RSSICache {
    esp_bd_addr_t address;
    int8_t rssi;
    unsigned long lastUpdate;
    bool valid;
};

struct DeviceNameCache {
    esp_bd_addr_t address;
    char name[32];
    bool hasCustomName;
    bool nvsStored;
    bool valid;
};

struct DevicePriorityCache {
    esp_bd_addr_t address;
    bool isPriority;
    bool nvsStored;
    bool valid;
};

#define RSSI_CACHE_SIZE 10
#define NAME_CACHE_SIZE 10
#define PRIORITY_CACHE_SIZE 10
RSSICache rssiCache[RSSI_CACHE_SIZE];
DeviceNameCache nameCache[NAME_CACHE_SIZE];
DevicePriorityCache priorityCache[PRIORITY_CACHE_SIZE];

// Memory-efficient device management
static esp_ble_bond_dev_t sharedBondedDevicesBuffer[MAX_BONDS];
static bool sharedBufferInUse = false;
static char cachedDevicesJson[2048] = "";
static unsigned long lastJsonUpdate = 0;
static int lastDeviceCount = -1;
static bool jsonCacheValid = false;



// ========================================
// TIMING VARIABLES
// ========================================
unsigned long startRelayPulseStart = 0;
bool startRelayPulsing = false;
unsigned long lastButtonReleaseTime = 0;
int buttonPressStep = 0;
#define BUTTON_STEP_TIMEOUT 2000

// Debug macros now defined at top of file

// ========================================
// MORE SYSTEM VARIABLES
// ========================================

// Button and input tracking
unsigned long lastButtonPress = 0;
unsigned long lastBrakePress = 0;
unsigned long startRelayTimer = 0;
bool startRelayActive = false;
bool engineRunning = false;
int systemState = 0;
const unsigned long START_RELAY_TIME = 700;
bool lastButtonReading = HIGH;
bool lastBrakeReading = HIGH;
bool brakeHeld = false;
bool buttonPressed = false;
unsigned long buttonPressStartTime = 0;
bool isLongPressDetected = false;

// Security and timing
bool securityEnabled = false;
unsigned long lastSecurityCheck = 0;
unsigned long lastEngineShutdown = 0;
unsigned long autoLockTimeout = AUTO_LOCK_TIMEOUT;
unsigned long starterPulseTime = STARTER_PULSE_TIME;
unsigned long lastShutdownTime = 0;
bool isShuttingDown = false;

// LED control
int ledBrightness = 0;
int ledFadeAmount = 5;
#define LED_PWM_FREQ 5000
#define LED_PWM_CHANNEL 0
#define LED_PWM_RESOLUTION 8
#define LED_PWM_DUTY_CYCLE 255

// WiFi configuration
const char* ap_ssid = "Ghost Key Configuration";
String ap_password = "123456789"; // Default password, will be loaded from preferences
String web_password = "1234"; // Default web interface password, will be loaded from preferences
bool wifiEnabled = false;

// More timing constants
#define CONFIG_MODE_PRESS_TIME 10000
#define STATUS_PRINT_INTERVAL 5000
#define SECURITY_CHECK_INTERVAL 1000
#define SHUTDOWN_DELAY 1000
#define FACTORY_RESET_TIME 30000

// Factory reset tracking
unsigned long factoryResetStartTime = 0;
bool factoryResetInProgress = false;

// Statistical RSSI Analysis global instance (confidence-based authentication)
RSSIAnalysis rssiAnalysis;

// Confidence momentum tracking for smoothing
static float lastConfidence = 0.0f;

// Calibration system variables
bool isCalibrating = false;
unsigned long calibrationStartTime = 0;
unsigned long calibrationDuration = 30000; // 30 seconds
float calibrationOffset = 0.0f; // Offset to add to confidence
#define MAX_CALIBRATION_SAMPLES 300 // 30 seconds at ~100ms intervals
float calibrationRSSIReadings[MAX_CALIBRATION_SAMPLES];
float calibrationConfidenceReadings[MAX_CALIBRATION_SAMPLES];
int calibrationSampleCount = 0;

// ========================================
// BLUETOOTH FUNCTIONS
// ========================================

// cleanupNVSStorage - Clears our custom device names/settings but keeps BLE bonds
// Used when: resetting device names but keeping paired devices
// Links to: saveDeviceName(), getDeviceName() functions
void cleanupNVSStorage() {
    Serial.println("=== STARTING CUSTOM NVS CLEANUP ===");
    
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        Serial.println("Erasing our custom namespace only...");
        nvs_erase_all(handle);
        nvs_commit(handle);
        nvs_close(handle);
        Serial.println("Custom namespace cleaned successfully");
    } else {
        Serial.printf("Could not open custom namespace for cleanup: %d\n", err);
    }
    
    Serial.println("=== CUSTOM NVS CLEANUP COMPLETE ===");
    Serial.println("Note: BLE bonding data preserved");
}

// getBondedDevicesSafe - Gets list of paired devices, handles memory safely
// Takes: int* count (returns number of devices found)
// Returns: esp_ble_bond_dev_t* (array of bonded devices)
// Links to: Used by getDevicesJson(), GAP event handler for whitelist checks
esp_ble_bond_dev_t* getBondedDevicesSafe(int* count) {
    if (sharedBufferInUse) {
        int bondedCount = esp_ble_get_bond_device_num();
        if (bondedCount == 0) {
            *count = 0;
            return nullptr;
        }
        esp_ble_bond_dev_t* tempBuffer = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
        if (tempBuffer) {
            esp_ble_get_bond_device_list(&bondedCount, tempBuffer);
            *count = bondedCount;
        }
        return tempBuffer;
    } else {
        sharedBufferInUse = true;
        int bondedCount = esp_ble_get_bond_device_num();
        if (bondedCount == 0) {
            *count = 0;
            sharedBufferInUse = false;
            return nullptr;
        }
        esp_ble_get_bond_device_list(&bondedCount, sharedBondedDevicesBuffer);
        *count = bondedCount;
        return sharedBondedDevicesBuffer;
    }
}

// releaseBondedDevicesBuffer - Cleans up memory from getBondedDevicesSafe()
// Takes: esp_ble_bond_dev_t* buffer (the buffer to release)
// Links to: Must be called after every getBondedDevicesSafe() call
void releaseBondedDevicesBuffer(esp_ble_bond_dev_t* buffer) {
    if (buffer == sharedBondedDevicesBuffer) {
        sharedBufferInUse = false;
    } else if (buffer != nullptr) {
        free(buffer);
    }
}

// invalidateDeviceCache - Forces web interface to rebuild device list
// Used when: device is added/removed, names changed
// Links to: Called by web server endpoints, saveDeviceName()
void invalidateDeviceCache() {
    jsonCacheValid = false;
    cachedDevicesJson[0] = '\0';
}

// Function to find or create name cache entry
int findOrCreateNameCacheEntry(esp_bd_addr_t address) {
    // First, look for existing entry
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (nameCache[i].valid && 
            memcmp(nameCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            return i;
        }
    }
    
    // Look for empty slot
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (!nameCache[i].valid) {
            memcpy(nameCache[i].address, address, sizeof(esp_bd_addr_t));
            strcpy(nameCache[i].name, "Unknown Device");
            nameCache[i].hasCustomName = false;
            nameCache[i].nvsStored = false;
            nameCache[i].valid = true;
            return i;
        }
    }
    
    // No empty slot, replace oldest entry
    memcpy(nameCache[0].address, address, sizeof(esp_bd_addr_t));
    strcpy(nameCache[0].name, "Unknown Device");
    nameCache[0].hasCustomName = false;
    nameCache[0].nvsStored = false;
    nameCache[0].valid = true;
    return 0;
}

// Function to save device name to cache and conditionally to NVS
void saveDeviceName(esp_bd_addr_t address, const char* name) {
    int cacheIndex = findOrCreateNameCacheEntry(address);
    
    // Update cache
    strncpy(nameCache[cacheIndex].name, name, sizeof(nameCache[cacheIndex].name) - 1);
    nameCache[cacheIndex].name[sizeof(nameCache[cacheIndex].name) - 1] = '\0';
    nameCache[cacheIndex].hasCustomName = true;
    
    // Save user-set custom names to NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        char key[32];  // Reduced from 64 to 32 (sufficient for key)
        snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
                 DEVICE_NAME_KEY, address[0], address[1], address[2], 
                 address[3], address[4], address[5]);

        err = nvs_set_str(handle, key, name);
        if (err == ESP_OK) {
            nvs_commit(handle);
            nameCache[cacheIndex].nvsStored = true;
            Serial.printf("Device name saved: %s\n", name);
        }
        nvs_close(handle);
    }
}

// Function to get device name
bool getDeviceName(esp_bd_addr_t address, char* name, size_t max_len) {
    // First check cache
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (nameCache[i].valid && 
            memcmp(nameCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            strncpy(name, nameCache[i].name, max_len - 1);
            name[max_len - 1] = '\0';
            return true;
        }
    }
    
    // Not in cache, try to load from NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return false;
    }

    char key[32];  // Reduced from 64 to 32 (sufficient for key)
    snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
             DEVICE_NAME_KEY, address[0], address[1], address[2], 
             address[3], address[4], address[5]);

    size_t required_size = max_len;
    err = nvs_get_str(handle, key, name, &required_size);
    nvs_close(handle);
    
    if (err == ESP_OK) {
        // Store in cache for future access
        int cacheIndex = findOrCreateNameCacheEntry(address);
        strncpy(nameCache[cacheIndex].name, name, sizeof(nameCache[cacheIndex].name) - 1);
        nameCache[cacheIndex].name[sizeof(nameCache[cacheIndex].name) - 1] = '\0';
        nameCache[cacheIndex].hasCustomName = true;
        nameCache[cacheIndex].nvsStored = true;
        return true;
    }
    
    return false;
}

// Function to validate RSSI value
bool isRSSIValid(int8_t rssi) {
    return (rssi >= RSSI_MIN_VALID && rssi <= RSSI_MAX_VALID && rssi != 0);
}

// Function to save device RSSI in memory cache only
void saveDeviceRSSI(esp_bd_addr_t address, int8_t rssi) {
    // Validate RSSI value before storing
    if (!isRSSIValid(rssi)) {
        Serial.printf("BLE: Invalid RSSI value %d dBm from device %02x:%02x:%02x:%02x:%02x:%02x - ignoring\n", 
                     rssi, address[0], address[1], address[2], address[3], address[4], address[5]);
        return;
    }
    
    int targetIndex = -1;
    unsigned long oldestTime = ULONG_MAX;
    
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        if (!rssiCache[i].valid) {
            targetIndex = i;
            break;
        }
        
        if (memcmp(rssiCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            targetIndex = i;
            break;
        }
        
        if (rssiCache[i].lastUpdate < oldestTime) {
            oldestTime = rssiCache[i].lastUpdate;
            targetIndex = i;
        }
    }
    
    if (targetIndex >= 0) {
        memcpy(rssiCache[targetIndex].address, address, sizeof(esp_bd_addr_t));
        rssiCache[targetIndex].rssi = rssi;
        rssiCache[targetIndex].lastUpdate = millis();
        rssiCache[targetIndex].valid = true;
        
        // Log RSSI updates only for significant changes (>5 dBm difference) to reduce spam
        static int8_t lastLoggedRSSI = RSSI_INVALID_VALUE;
        static esp_bd_addr_t lastLoggedAddr = {0};
        if (memcmp(lastLoggedAddr, address, sizeof(esp_bd_addr_t)) != 0 || 
            abs(rssi - lastLoggedRSSI) > 5) {
            Serial.printf("BLE: RSSI cached: %d dBm (valid range: %d to %d dBm)\n", 
                         rssi, RSSI_MIN_VALID, RSSI_MAX_VALID);
            lastLoggedRSSI = rssi;
            memcpy(lastLoggedAddr, address, sizeof(esp_bd_addr_t));
        }
    } else {
        Serial.println("BLE: Warning - RSSI cache full, could not store RSSI value");
    }
}

// Function to get device RSSI from memory cache
bool getDeviceRSSI(esp_bd_addr_t address, int8_t* rssi) {
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        if (rssiCache[i].valid && 
            memcmp(rssiCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            
            // Check if data is recent (within last 30 seconds)
            if (millis() - rssiCache[i].lastUpdate < 30000) {
                *rssi = rssiCache[i].rssi;
                return true;
            } else {
                rssiCache[i].valid = false;
                break;
            }
        }
    }
    return false;
}

// calculateDistance - Converts RSSI signal strength to approximate distance
// Takes: int8_t rssi (signal strength in dBm)
// Returns: float distance (meters, or -1.0 if invalid)
// Links to: Used by getDevicesJson() for web interface display
float calculateDistance(int8_t rssi) {
    if (rssi == 0 || rssi >= -10) {
        return -1.0;
    }
    
    if (rssi >= -50) return 1.0;
    else if (rssi >= -65) return 2.0;
    else if (rssi >= -68) return 4.0;
    else if (rssi >= -73) return 5.0;
    else if (rssi >= -80) return 8.0;
    else return 12.0;
}

// BLE Server Callback Class
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("BLE: Device attempting to connect...");
        
        // If not in pairing mode, check will be done in GAP event handler
        if (!isPairingMode) {
            Serial.println("BLE: Not in pairing mode - will verify during authentication");
        } else {
            Serial.println("BLE: In pairing mode - accepting connection from any device");
        }
        
        // Accept connection initially, but don't set hasConnectedDevice until authentication
        Serial.println("BLE: Device connected - waiting for authentication");
        isBleConnected = true;
        // hasConnectedDevice will be set in GAP event handler after successful authentication
        
        // Visual feedback
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
    }
    
    void onDisconnect(BLEServer* pServer) {
        Serial.println("BLE: Device disconnected");
        
        hasConnectedDevice = false;
        isBleConnected = false;
        bluetoothAuthenticated = false;
        
        // Clear connected device address
        memset(connectedDeviceAddr, 0, sizeof(connectedDeviceAddr));
        
        invalidateDeviceCache();
        
        // Restart advertising after disconnect
        Serial.println("BLE: Restarting advertising after disconnect...");
        startBLEAdvertising(isPairingMode);
    }
};

// Function to initialize Bluetooth
void initializeBluetooth() {
    Serial.println("=== INITIALIZING BLUETOOTH ===");
    
    // Initialize BLE
    Serial.println("BLE: Initializing BLE Device...");
    BLEDevice::init("Ghost-Key Secure");
    Serial.println("BLE: Device initialized with name: Ghost-Key Secure");
    
    // Set BLE power to maximum for better range
    Serial.println("BLE: Setting power levels...");
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
    Serial.println("BLE: Power levels set to maximum");
    
    // Create BLE Server
    Serial.println("BLE: Creating BLE Server...");
    pServer = BLEDevice::createServer();
    
    // Set server callbacks
    pServer->setCallbacks(new MyServerCallbacks());
    Serial.println("BLE: Server callbacks set");
    
    // Set up security with bonding
    Serial.println("BLE: Setting up security...");
    BLESecurity* pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    pSecurity->setCapability(ESP_IO_CAP_NONE);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    Serial.println("BLE: Security configured with bonding");
    
    // Start keyboard (this handles its own advertising)
    Serial.println("BLE: Starting BLE Keyboard...");
    bleKeyboard.begin();
    Serial.println("BLE: Keyboard started");
    
    // Register GAP callback for events
    esp_ble_gap_register_callback(onGapEvent);
    Serial.println("BLE: GAP callback registered");
    
    // BLE advertising will be started separately
    Serial.println("BLE: Ready to start advertising");
    
    // Print bonded devices count
    int bondedCount = esp_ble_get_bond_device_num();
    Serial.printf("BLE: Found %d bonded devices\n", bondedCount);
    
    // Start advertising in normal mode initially
    startBLEAdvertising(false);
    
    Serial.println("=== BLUETOOTH INITIALIZATION COMPLETE ===");
}

// Function to start BLE advertising with proper discoverability
void startBLEAdvertising(bool discoverable) {
    if(pServer == nullptr) {
        Serial.println("BLE: ERROR - Server not initialized");
        return;
    }
    
    // Check if we can accept new bonds
    int currentBonds = esp_ble_get_bond_device_num();
    if (discoverable && currentBonds >= MAX_BONDS) {
        Serial.println("BLE: Cannot enter pairing mode - maximum bonds reached!");
        Serial.println("BLE: Please remove a device first.");
        isPairingMode = false;
        return;
    }
    
    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->stop();
    delay(100); // Give time for stop to complete
    
    if(discoverable) {
        Serial.println("BLE: Setting up advertising for PAIRING MODE:");
        Serial.println("BLE: - Fast advertising for discovery");
        Serial.println("BLE: - General discoverable mode");
        Serial.println("BLE: - Interval: 20ms to 40ms");
        Serial.println("BLE: - Allowing ALL devices to connect");
        
        // Set up advertising for pairing mode - accept all devices
        pAdvertising->setAdvertisementType(esp_ble_adv_type_t::ADV_TYPE_IND);
        pAdvertising->setScanResponse(true);
        pAdvertising->setAppearance(0x03C1);  // Keyboard appearance
        
        // Create advertising data with proper flags for discoverability
        BLEAdvertisementData adv_data;
        adv_data.setFlags(0x06);  // General discoverable + BR/EDR not supported
        adv_data.setCompleteServices(BLEUUID((uint16_t)0x1812)); // HID Service
        adv_data.setName("Ghost-Key Secure");
        pAdvertising->setAdvertisementData(adv_data);
        
        // Create scan response data
        BLEAdvertisementData scan_data;
        scan_data.setManufacturerData("Ghost-Key Inc.");
        scan_data.setName("Ghost-Key Secure");
        pAdvertising->setScanResponseData(scan_data);
        
        // Fast advertising for pairing
        pAdvertising->setMinInterval(0x20);  // 20ms
        pAdvertising->setMaxInterval(0x40);  // 40ms
        
    } else {
        Serial.println("BLE: Setting up advertising for NORMAL MODE:");
        Serial.println("BLE: - Slow advertising to save power");
        Serial.println("BLE: - Limited discoverable mode");
        Serial.println("BLE: - Interval: 160ms to 320ms");
        Serial.println("BLE: - Only bonded devices allowed");
        
        // Set up advertising for normal mode
        pAdvertising->setAdvertisementType(esp_ble_adv_type_t::ADV_TYPE_IND);
        pAdvertising->setScanResponse(true);
        pAdvertising->setAppearance(0x03C1);  // Keyboard appearance
        
        // Create advertising data with limited discoverability
        BLEAdvertisementData adv_data;
        adv_data.setFlags(0x05);  // Limited discoverable + BR/EDR not supported
        adv_data.setCompleteServices(BLEUUID((uint16_t)0x1812)); // HID Service
        adv_data.setName("Ghost-Key Secure");
        pAdvertising->setAdvertisementData(adv_data);
        
        // Slower advertising for normal operation
        pAdvertising->setMinInterval(0x100); // 160ms
        pAdvertising->setMaxInterval(0x200); // 320ms
    }
    
    pAdvertising->start();
    Serial.println("BLE: Advertising started successfully - Device should be discoverable");
    if (discoverable) {
        Serial.println("BLE: *** DEVICE IS NOW VISIBLE ON IPHONE ***");
        Serial.println("BLE: Look for 'Ghost-Key Secure' in Bluetooth settings");
    }
}

// Function to stop BLE advertising  
void stopBLEAdvertising() {
    Serial.println("BLE: Stopping advertising...");
    
    if(pServer == nullptr) return;
    
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->stop();
    Serial.println("BLE: Advertising stopped successfully");
}

// Enhanced GAP event handler for debugging
void onGapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
                // Record first RSSI scan completion time
                if (!firstRSSIScanDone) {
                    firstRSSIScanCompleteTime = millis();
                    firstRSSIScanDone = true;
                    Serial.println("=== FIRST RSSI SCAN RESULT RECEIVED ===");
                    Serial.print("First RSSI scan complete at: ");
                    Serial.print(firstRSSIScanCompleteTime);
                    Serial.println("ms");
                    Serial.print("Time from boot to first RSSI result: ");
                    Serial.print(firstRSSIScanCompleteTime - systemBootTime);
                    Serial.println("ms");
                    Serial.print("Time from scan start to first result: ");
                    Serial.print(firstRSSIScanCompleteTime - firstRSSIScanStartTime);
                    Serial.println("ms");
                    Serial.print("Time from BT init to first RSSI result: ");
                    Serial.print(firstRSSIScanCompleteTime - bluetoothInitCompleteTime);
                    Serial.println("ms");
                    Serial.println("=== BLUETOOTH TIMING SUMMARY ===");
                    Serial.print("Boot to BT init start: ");
                    Serial.print(bluetoothInitStartTime - systemBootTime);
                    Serial.println("ms");
                    Serial.print("BT initialization duration: ");
                    Serial.print(bluetoothInitCompleteTime - bluetoothInitStartTime);
                    Serial.println("ms");
                    Serial.print("BT init to RSSI scan start: ");
                    Serial.print(firstRSSIScanStartTime - bluetoothInitCompleteTime);
                    Serial.println("ms");
                    Serial.print("RSSI scan to first result: ");
                    Serial.print(firstRSSIScanCompleteTime - firstRSSIScanStartTime);
                    Serial.println("ms");
                    Serial.print("TOTAL: Boot to first RSSI result: ");
                    Serial.print(firstRSSIScanCompleteTime - systemBootTime);
                    Serial.println("ms");
                    Serial.println("=====================================");
                }
                
                // Update RSSI for bonded devices (silent update)
                int bondedCount = esp_ble_get_bond_device_num();
                if (bondedCount > 0) {
                    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
                    if (bondedDevices) {
                        for (int i = 0; i < bondedCount; i++) {
                            if (memcmp(bondedDevices[i].bd_addr, param->scan_rst.bda, sizeof(esp_bd_addr_t)) == 0) {
                                // Validate RSSI before processing
                                if (isRSSIValid(param->scan_rst.rssi)) {
                                    // Add to confidence-based statistical analysis system
                                    addRSSIReading(param->scan_rst.rssi, param->scan_rst.bda);
                                    // Keep RSSI caching for web interface
                                    saveDeviceRSSI(param->scan_rst.bda, param->scan_rst.rssi);
                                } else {
                                    Serial.printf("BLE: Discarded invalid RSSI %d dBm from scan result\n", param->scan_rst.rssi);
                                }
                                break;
                            }
                        }
                        releaseBondedDevicesBuffer(bondedDevices);
                    }
                }
            }
            break;

        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            if (param->ble_security.auth_cmpl.success) {
                Serial.println("BLE: Authentication successful!");
                
                // Get the authenticated device address
                esp_bd_addr_t addr;
                memcpy(addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
                memcpy(connectedDeviceAddr, addr, sizeof(esp_bd_addr_t));
                
                Serial.printf("BLE: Authenticated device: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
                
                hasConnectedDevice = true;
                
                // Visual feedback for successful authentication
                for(int i = 0; i < 3; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(200);
                    digitalWrite(LED_PIN, LOW);
                    delay(200);
                }
                
            } else {
                Serial.printf("BLE: Authentication failed, error: %d\n", param->ble_security.auth_cmpl.fail_reason);
                if (pServer != nullptr) {
                    pServer->disconnect(pServer->getConnId());
                }
            }
            break;
            
        case ESP_GAP_BLE_SEC_REQ_EVT:
            {
                esp_bd_addr_t addr;
                memcpy(addr, param->ble_security.ble_req.bd_addr, sizeof(esp_bd_addr_t));
                
                Serial.printf("BLE: Security request from: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
                
                if (isPairingMode) {
                    // In pairing mode - accept any device (up to bond limit)
                    Serial.println("BLE: In pairing mode - accepting security request from any device");
                    esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
                } else {
                    // NOT in pairing mode - only allow whitelisted (bonded) devices
                    bool isDeviceBonded = false;
                    int bondedCount = esp_ble_get_bond_device_num();
                    if (bondedCount > 0) {
                        esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
                        if (bondedDevices) {
                            for (int i = 0; i < bondedCount; i++) {
                                if (memcmp(bondedDevices[i].bd_addr, addr, sizeof(esp_bd_addr_t)) == 0) {
                                    isDeviceBonded = true;
                                    break;
                                }
                            }
                            releaseBondedDevicesBuffer(bondedDevices);
                        }
                    }
                    
                    if (isDeviceBonded) {
                        Serial.println("BLE: Device is whitelisted - accepting security request");
                        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
                    } else {
                        Serial.println("BLE: Device NOT whitelisted - rejecting security request");
                        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, false);
                        // Disconnect the device immediately
                        if (pServer != nullptr) {
                            pServer->disconnect(pServer->getConnId());
                        }
                    }
                }
            }
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                Serial.println("BLE: Advertising started successfully");
            } else {
                Serial.printf("BLE: Advertising start failed: %d\n", param->adv_start_cmpl.status);
            }
            break;

        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                Serial.println("BLE: Advertising stopped successfully");
            } else {
                Serial.printf("BLE: Advertising stop failed: %d\n", param->adv_stop_cmpl.status);
            }
            break;

        default:
            Serial.printf("BLE: GAP event: %d\n", event);
            break;
    }
}

// Function to toggle pairing mode with proper advertising control
void toggleBluetoothPairingMode() {
    isPairingMode = !isPairingMode;
    
    Serial.println("=== BLUETOOTH PAIRING MODE TOGGLE ===");
    Serial.printf("Pairing mode is now: %s\n", isPairingMode ? "ACTIVE" : "INACTIVE");
    
    if (isPairingMode) {
        pairingModeStartTime = millis();  // Record when pairing mode started
        Serial.println("PAIRING: Entering pairing mode...");
        Serial.println("PAIRING: Making device discoverable...");
        Serial.printf("PAIRING: Auto-timeout in %d seconds\n", PAIRING_MODE_TIMEOUT / 1000);
        
        // Restart advertising to make device discoverable
        stopBLEAdvertising();
        delay(100);
        startBLEAdvertising(true);
        
        Serial.println("PAIRING: *** DEVICE SHOULD NOW BE VISIBLE ON IPHONE ***");
        Serial.println("PAIRING: Look for 'Ghost-Key Secure' in Bluetooth settings");
        Serial.println("PAIRING: Device name: Ghost-Key Secure");
        Serial.println("PAIRING: Manufacturer: Ghost-Key Inc.");
        
    } else {
        pairingModeStartTime = 0;  // Clear the start time
        Serial.println("PAIRING: Exiting pairing mode...");
        Serial.println("PAIRING: Switching to bonded devices only mode");
        
        // Switch to normal advertising mode
        stopBLEAdvertising();
        delay(100);
        startBLEAdvertising(false);
    }
    
    Serial.println("=== PAIRING MODE TOGGLE COMPLETE ===");
}

// Function to disable pairing mode (for timeout and safety)
void disablePairingMode() {
    if (isPairingMode) {
        Serial.println("PAIRING: Disabling pairing mode (timeout/safety)");
        isPairingMode = false;
        pairingModeStartTime = 0;
        
        // Switch to normal advertising mode
        stopBLEAdvertising();
        delay(100);
        startBLEAdvertising(false);
    }
}

// Function to start RSSI scanning
void startRSSIScan() {
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,  // Reduced interval for more frequent updates
        .scan_window = 0x30,    // Increased window for better reception
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    esp_ble_gap_set_scan_params(&scan_params);
    esp_ble_gap_start_scanning(0);
    // Reduced logging - only log once at startup
    static bool firstScan = true;
    if (firstScan) {
        Serial.println("BLE: RSSI scanning started");
        firstScan = false;
    }
}

// Function to stop RSSI scanning
void stopRSSIScan() {
    esp_ble_gap_stop_scanning();
    // Reduced logging - comment out the stop message
    // Serial.println("BLE: RSSI scanning stopped");
}

// Function to properly shutdown Bluetooth
void shutdownBluetooth() {
    if (!bluetoothInitialized) {
        return;
    }
    
    Serial.println("=== SHUTTING DOWN BLUETOOTH ===");
    
    // Stop any ongoing operations first
    stopRSSIScan();
    
    // Reset state variables
    isBleConnected = false;
    bluetoothAuthenticated = false;
    hasConnectedDevice = false;
    isPairingMode = false;
    pairingModeStartTime = 0;
    
    // Clear connected device address
    memset(connectedDeviceAddr, 0, sizeof(connectedDeviceAddr));
    
    // Stop advertising safely
    if (pServer != nullptr) {
        stopBLEAdvertising();
        delay(100); // Give time for advertising to stop
    }
    
    // Mark as not initialized but don't deinitialize aggressively
    bluetoothInitialized = false;
    
    // Reset RSSI analysis when Bluetooth is disabled
    resetRSSIAnalysis();
    
    Serial.println("=== BLUETOOTH SHUTDOWN COMPLETE ===");
}

// Function to restart Bluetooth after shutdown
void restartBluetooth() {
    if (bluetoothInitialized) {
        return;
    }
    
    Serial.println("=== RESTARTING BLUETOOTH ===");
    
    // Small delay to ensure clean state
    delay(200);
    
    // Reinitialize Bluetooth (this will handle BLE setup)
    initializeBluetooth();
    bluetoothInitialized = true;
    
    Serial.println("=== BLUETOOTH RESTART COMPLETE ===");
}

// ========================================
// RFID FUNCTIONS
// ========================================

// Manchester decode. Supply the function an array to store the tags ID in
bool decodeTag(unsigned char *buf)
{
  unsigned char i = 0;
  unsigned short timeCount;
  unsigned char timeOutFlag = 0;
  unsigned char row, col;
  unsigned char row_parity;
  unsigned char col_parity[5];
  unsigned char dat;
  unsigned char j;
  
  while(1)
  {
    timeCount = 0;
    while(0 == digitalRead(RFID_DEMOD_OUT)) // watch for demodOut to go low
    {
      if(timeCount >= RFID_TIMEOUT) // if we pass TIMEOUT milliseconds, break out of the loop
      {
        break;
      }
      else
      {
        timeCount++;
      }
    }

    if (timeCount >= 600)
    {
      return false;
    }
    timeCount = 0;

    delayMicroseconds(RFID_DELAYVAL);
    if(digitalRead(RFID_DEMOD_OUT))
    {
      for(i = 0; i < 8; i++) // 9 header bits
      {
        timeCount = 0; // restart counting
        while(1 == digitalRead(RFID_DEMOD_OUT)) // while DEMOD out is high
        {
          if(timeCount == RFID_TIMEOUT)
          {
            timeOutFlag = 1;
            break;
          }
          else
          {
            timeCount++;
          }
        }

        if(timeOutFlag)
        {
          break;
        }
        else
        {
          delayMicroseconds(RFID_DELAYVAL);
          if( 0 == digitalRead(RFID_DEMOD_OUT) )
          {
            break;
          }
        }
      } // end for loop

      if(timeOutFlag)
      {
        timeOutFlag = 0;
        return false;
      }

      if(i == 8) // Receive the data
      {
        timeOutFlag = 0;
        timeCount = 0;
        while(1 == digitalRead(RFID_DEMOD_OUT))
        {
          if(timeCount == RFID_TIMEOUT)
          {
            timeOutFlag = 1;
            break;
          }
          else
          {
            timeCount++;
          }

          if(timeOutFlag)
          {
            timeOutFlag = 0;
            return false;
          }
        }

        col_parity[0] = col_parity[1] = col_parity[2] = col_parity[3] = col_parity[4] = 0;
        for(row = 0; row < 11; row++)
        {
          row_parity = 0;
          j = row >> 1;

          for(col = 0, row_parity = 0; col < 5; col++)
          {
            delayMicroseconds(RFID_DELAYVAL);
            if(digitalRead(RFID_DEMOD_OUT))
            {
              dat = 1;
            }
            else
            {
              dat = 0;
            }

            if(col < 4 && row < 10)
            {
              buf[j] <<= 1;
              buf[j] |= dat;
            }

            row_parity += dat;
            col_parity[col] += dat;
            timeCount = 0;
            while(digitalRead(RFID_DEMOD_OUT) == dat)
            {
              if(timeCount == RFID_TIMEOUT)
              {
                timeOutFlag = 1;
                break;
              }
              else
              {
                timeCount++;
              }
            }
            if(timeOutFlag)
            {
              break;
            }
          }

          if(row < 10)
          {
            if((row_parity & 0x01) || timeOutFlag) // Row parity
            {
              timeOutFlag = 1;
              break;
            }
          }
        }

        if( timeOutFlag || (col_parity[0] & 0x01) || (col_parity[1] & 0x01) || (col_parity[2] & 0x01) || (col_parity[3] & 0x01) ) // Column parity
        {
          timeOutFlag = 0;
          return false;
        }
        else
        {
          return true;
        }
      } // end if(i==8)

      return false;
    } // if(digitalRead(RFID_DEMOD_OUT))
  } // while(1)
}

// Function to compare 2 byte arrays. Returns true if the two arrays match, false of any numbers do not match
bool compareTagData(byte *tagData1, byte *tagData2)
{
  for(int j = 0; j < 5; j++)
  {
    if (tagData1[j] != tagData2[j])
    {
      return false; // if any of the ID numbers are not the same, return a false
    }
  }
  return true;  // all id numbers have been verified
}

// Function to transfer one byte array to a secondary byte array.
// source -> tagData
// destination -> tagDataBuffer
void transferToBuffer(byte *tagData, byte *tagDataBuffer)
{
  for(int j = 0; j < 5; j++)
  {
    tagDataBuffer[j] = tagData[j];
  }
}

bool scanForTag(byte *tagData)
{
  static byte tagDataBuffer[5];      // A Buffer for verifying the tag data. 'static' so that the data is maintained the next time the loop is called
  static int readCount = 0;          // the number of times a tag has been read. 'static' so that the data is maintained the next time the loop is called
  boolean verifyRead = false; // true when a tag's ID matches a previous read, false otherwise
  boolean tagCheck = false;   // true when a tag has been read, false otherwise

  tagCheck = decodeTag(tagData); // run the decodetag to check for the tag
  if (tagCheck == true) // if 'true' is returned from the decodetag function, a tag was succesfully scanned
  {
    readCount++;      // increase count since we've seen a tag

    if(readCount == 1) // if have read a tag only one time, proceed
    {
      transferToBuffer(tagData, tagDataBuffer);  // place the data from the current tag read into the buffer for the next read
    }
    else if(readCount == 2) // if we see a tag a second time, proceed
    {
      verifyRead = compareTagData(tagData, tagDataBuffer); // run the checkBuffer function to compare the data in the buffer (the last read) with the data from the current read

      if (verifyRead == true) // if a 'true' is returned by compareTagData, the current read matches the last read
      {
        readCount = 0; // because a tag has been succesfully verified, reset the readCount to '0' for the next tag
        return true;
      }
    }
  }
  else
  {
    return false;
  }
  return true;
}

// ========================================
// RFID KEY MANAGEMENT FUNCTIONS
// ========================================

// Load stored RFID keys from preferences
void loadStoredRfidKeys() {
    numStoredKeys = preferences.getInt("rfid_count", 0);
    if (numStoredKeys > MAX_RFID_KEYS) {
        numStoredKeys = 0;  // Reset if corrupted
    }
    
    for (int i = 0; i < numStoredKeys; i++) {
        String keyName = "rfid_key_" + String(i);
        size_t keySize = preferences.getBytesLength(keyName.c_str());
        if (keySize == 5) {
            preferences.getBytes(keyName.c_str(), storedRfidKeys[i], 5);
        } else {
            numStoredKeys = i;  // Stop if we hit corrupted data
            break;
        }
    }
    
    Serial.printf("Loaded %d RFID keys from storage\n", numStoredKeys);
}

// Save stored RFID keys to preferences
void saveStoredRfidKeys() {
    preferences.putInt("rfid_count", numStoredKeys);
    
    for (int i = 0; i < numStoredKeys; i++) {
        String keyName = "rfid_key_" + String(i);
        preferences.putBytes(keyName.c_str(), storedRfidKeys[i], 5);
    }
    
    Serial.printf("Saved %d RFID keys to storage\n", numStoredKeys);
}

// Add a new RFID key to storage
bool addRfidKey(byte *newKey) {
    if (numStoredKeys >= MAX_RFID_KEYS) {
        Serial.println("RFID: Maximum keys reached, cannot add more");
        return false;
    }
    
    // Check if key already exists
    for (int i = 0; i < numStoredKeys; i++) {
        if (compareTagData(storedRfidKeys[i], newKey)) {
            Serial.println("RFID: Key already exists");
            return false;
        }
    }
    
    // Add new key
    for (int i = 0; i < 5; i++) {
        storedRfidKeys[numStoredKeys][i] = newKey[i];
    }
    numStoredKeys++;
    
    saveStoredRfidKeys();
    
    Serial.print("RFID: Added new key: ");
    for (int i = 0; i < 5; i++) {
        Serial.print(newKey[i], DEC);
        if (i < 4) Serial.print(",");
    }
    Serial.println();
    
    return true;
}

// Remove an RFID key by index
bool removeRfidKey(int index) {
    if (index < 0 || index >= numStoredKeys) {
        return false;
    }
    
    // Shift remaining keys down
    for (int i = index; i < numStoredKeys - 1; i++) {
        for (int j = 0; j < 5; j++) {
            storedRfidKeys[i][j] = storedRfidKeys[i + 1][j];
        }
    }
    
    numStoredKeys--;
    saveStoredRfidKeys();
    
    Serial.printf("RFID: Removed key at index %d\n", index);
    return true;
}

// Check if a tag matches any stored keys
bool checkRfidKey(byte *tagToCheck) {
    for (int i = 0; i < numStoredKeys; i++) {
        if (compareTagData(storedRfidKeys[i], tagToCheck)) {
            Serial.printf("RFID: Tag matched stored key %d\n", i);
            return true;
        }
    }
    return false;
}

// Get stored keys as JSON for web interface
String getRfidKeysJson() {
    String json = "[";
    
    for (int i = 0; i < numStoredKeys; i++) {
        if (i > 0) json += ",";
        json += "{\"index\":" + String(i) + ",\"id\":\"";
        for (int j = 0; j < 5; j++) {
            json += String(storedRfidKeys[i][j]);
            if (j < 4) json += ",";
        }
        json += "\"}";
    }
    
    json += "]";
    return json;
}

// ========================================
// FUNCTION DECLARATIONS
// ========================================
void setupPins();
void setupWiFi();
void setupWebServer();
void setupBluetooth();
void handleButtonPress();
void handleBrakeInput();
void updateSystemState();
void controlRelays();
void checkAutoLock();
void enterConfigMode();
void exitConfigMode();
void updateSecurityState();

// ========================================
// MAIN SETUP - Runs once on boot
// ========================================
void setup() {
    systemBootTime = millis();
    Serial.begin(115200);
    DEBUG_PRINTLN("\n\n=== GhostKey System Starting ===");
    DEBUG_PRINTLN("Initializing system...");
    DEBUG_PRINT("System boot time: ");
    DEBUG_PRINT(systemBootTime);
    DEBUG_PRINTLN("ms");
    
    // Hardware setup
    setupPins();
    DEBUG_PRINTLN("Pins initialized");
    
    // Load saved settings from flash
    preferences.begin("ghostkey", false);
    isConfigured = preferences.getBool("configured", false);
    isBluetoothPaired = preferences.getBool("bluetooth_paired", false);
    firstSetupComplete = preferences.getBool("first_setup_complete", false);
    starterPulseTime = preferences.getULong("starter_pulse", STARTER_PULSE_TIME);
    autoLockTimeout = preferences.getULong("auto_lock_timeout", AUTO_LOCK_TIMEOUT);
    ap_password = preferences.getString("wifi_password", "123456789");
    web_password = preferences.getString("web_password", "1234");
    bluetoothEnabled = preferences.getBool("bt_enabled", true);
    ghostKeyEnabled = preferences.getBool("ghost_key_enabled", true);
    ghostPowerEnabled = preferences.getBool("ghost_power_enabled", true);
    calibrationOffset = preferences.getFloat("calibration_offset", 0.0f);
    
    // Legacy RSSI calibration removed - using confidence-based authentication only
    
    DEBUG_PRINT("System configured: ");
    DEBUG_PRINTLN(isConfigured ? "Yes" : "No");
    DEBUG_PRINT("Bluetooth paired: ");
    DEBUG_PRINTLN(isBluetoothPaired ? "Yes" : "No");
    DEBUG_PRINT("First setup complete: ");
    DEBUG_PRINTLN(firstSetupComplete ? "Yes" : "No");
    DEBUG_PRINT("Starter pulse time: ");
    DEBUG_PRINT(starterPulseTime);
    DEBUG_PRINTLN("ms");
    DEBUG_PRINT("Auto-lock timeout: ");
    DEBUG_PRINT(autoLockTimeout);
    DEBUG_PRINTLN("ms");
    

    
    // Initialize NVS for bluetooth
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        Serial.printf("Error initializing NVS: %d\n", err);
    } else {
        Serial.println("NVS initialized for Bluetooth");
    }
    
    // SPIFFS removed - logo now served from PROGMEM
    
    // Initialize BLE cache arrays
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        rssiCache[i].valid = false;
    }
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        nameCache[i].valid = false;
    }
    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        priorityCache[i].valid = false;
    }
    
    // Initialize RSSI analysis system
    resetRSSIAnalysis();
    
    // Initialize BLE only if enabled
    if (bluetoothEnabled) {
    bluetoothInitStartTime = millis();
    DEBUG_PRINT("Starting Bluetooth initialization at: ");
    DEBUG_PRINT(bluetoothInitStartTime);
    DEBUG_PRINTLN("ms");
    
    initializeBluetooth();
        bluetoothInitialized = true;
    
    bluetoothInitCompleteTime = millis();
    DEBUG_PRINT("Bluetooth initialization complete at: ");
    DEBUG_PRINT(bluetoothInitCompleteTime);
    DEBUG_PRINTLN("ms");
    DEBUG_PRINT("Bluetooth init duration: ");
    DEBUG_PRINT(bluetoothInitCompleteTime - bluetoothInitStartTime);
    DEBUG_PRINTLN("ms");
    
    // Start RSSI scanning for bonded devices
    firstRSSIScanStartTime = millis();
    DEBUG_PRINT("Starting first RSSI scan at: ");
    DEBUG_PRINT(firstRSSIScanStartTime);
    DEBUG_PRINTLN("ms");
    DEBUG_PRINT("Time from boot to RSSI scan start: ");
    DEBUG_PRINT(firstRSSIScanStartTime - systemBootTime);
    DEBUG_PRINTLN("ms");
    
    startRSSIScan();
    DEBUG_PRINTLN("RSSI scanning started");
    } else {
        DEBUG_PRINTLN("Bluetooth disabled - skipping initialization");
        bluetoothInitialized = false;
    }
    
    // Load stored RFID keys
    loadStoredRfidKeys();
    DEBUG_PRINTLN("RFID keys loaded");
    
    DEBUG_PRINT("RFID system ready - Total init time: ");
    DEBUG_PRINT(rfidInitCompleteTime - rfidInitStartTime);
    DEBUG_PRINTLN("ms");
    
    DEBUG_PRINTLN("=== System Initialization Complete ===\n");
}

// ========================================
// MAIN LOOP - Runs continuously
// ========================================
void loop() {
    // Core system updates
    updateSystemState();
    
    // Handle authentication based on enabled systems
    if (ghostKeyEnabled) {
        // Ghost Key enabled: RFID/Bluetooth authentication and push-to-start
        if (bluetoothEnabled && bluetoothInitialized) {
    updateBluetoothAuthentication();
        }
        // RFID scanning handled in main loop below
        handleButtonPress(); // Push-to-start functionality
    } else {
        // Ghost Key disabled: Reset authentication states and disable wireless
        bluetoothAuthenticated = false;
        rfidAuthenticated = false;
        
        if (ghostPowerEnabled) {
            // Ghost Power only mode: Brake + Accessory authentication
            updateAccessoryAuthentication();
            // Configuration mode access still available via button
            handleConfigModeOnly(); // Only handle config mode button press
        }
    }
    
    // Control systems based on what's enabled
    if (ghostKeyEnabled) {
        controlRelays(); // Full relay control for push-to-start
    }
    if (ghostPowerEnabled) {
        // Security relay control handled in updateSecurityState()
    }
    
    checkAutoLock();
    
    // Periodic security check
    if (millis() - lastSecurityCheck >= SECURITY_CHECK_INTERVAL) {
        updateSecurityState();
        lastSecurityCheck = millis();
    }
    
    // Update BLE connection status (only if Bluetooth is enabled)
    if (bluetoothEnabled && bluetoothInitialized) {
    isBleConnected = bleKeyboard.isConnected();
    
    // RSSI scanning (only when needed)
    static unsigned long lastRSSIScan = 0;
    if (millis() - lastRSSIScan >= RSSI_UPDATE_INTERVAL) {
        int bondedCount = esp_ble_get_bond_device_num();
        if (bondedCount > 0 && (isBleConnected || isPairingMode)) {
            startRSSIScan();
        }
        lastRSSIScan = millis();
    }
        
        // Legacy RSSI calibration removed - using confidence-based authentication only;
    
    // Pairing mode timeout check
    if (isPairingMode && pairingModeStartTime > 0) {
        if (millis() - pairingModeStartTime >= PAIRING_MODE_TIMEOUT) {
            Serial.println("PAIRING: Timeout reached - disabling pairing mode");
            disablePairingMode();
        }
        }
    } else {
        // Reset Bluetooth state if disabled
        isBleConnected = false;
        bluetoothAuthenticated = false;
        isPairingMode = false;
    }
    
    // Web server handling
    if (currentState == CONFIG_MODE) {
        server.handleClient();
    }
    
    // Factory reset detection - start + brake for 30 seconds
    checkFactoryReset();
    
    // RFID scanning - only when Ghost Key is enabled
    if (ghostKeyEnabled) {
        // Track first scan timing
    static bool firstRfidScanStarted = false;
    if (!firstRfidScanStarted) {
        firstRfidScanStartTime = millis();
        firstRfidScanStarted = true;
        Serial.println("=== STARTING FIRST RFID SCAN ===");
        Serial.print("First RFID scan started at: ");
        Serial.print(firstRfidScanStartTime);
        Serial.println("ms");
        Serial.print("Time from boot to RFID scan start: ");
        Serial.print(firstRfidScanStartTime - systemBootTime);
        Serial.println("ms");
        Serial.print("Time from RFID init to scan start: ");
        Serial.print(firstRfidScanStartTime - rfidInitCompleteTime);
        Serial.println("ms");
        Serial.println("==============================");
    }
    
    if(scanForTag(tagData) == true) {
        // Record first RFID read completion time
        if (!firstRfidReadDone) {
            firstRfidReadCompleteTime = millis();
            firstRfidReadDone = true;
            Serial.println("=== FIRST RFID READ SUCCESSFUL ===");
            Serial.print("First RFID read complete at: ");
            Serial.print(firstRfidReadCompleteTime);
            Serial.println("ms");
            Serial.print("Time from boot to first RFID read: ");
            Serial.print(firstRfidReadCompleteTime - systemBootTime);
            Serial.println("ms");
            Serial.print("Time from scan start to first read: ");
            Serial.print(firstRfidReadCompleteTime - firstRfidScanStartTime);
            Serial.println("ms");
            Serial.print("Time from RFID init to first read: ");
            Serial.print(firstRfidReadCompleteTime - rfidInitCompleteTime);
            Serial.println("ms");
            Serial.println("=== RFID TIMING SUMMARY ===");
            Serial.print("Boot to RFID init start: ");
            Serial.print(rfidInitStartTime - systemBootTime);
            Serial.println("ms");
            Serial.print("RFID initialization duration: ");
            Serial.print(rfidInitCompleteTime - rfidInitStartTime);
            Serial.println("ms");
            Serial.print("RFID init to scan start: ");
            Serial.print(firstRfidScanStartTime - rfidInitCompleteTime);
            Serial.println("ms");
            Serial.print("RFID scan to first read: ");
            Serial.print(firstRfidReadCompleteTime - firstRfidScanStartTime);
            Serial.println("ms");
            Serial.print("TOTAL: Boot to first RFID read: ");
            Serial.print(firstRfidReadCompleteTime - systemBootTime);
            Serial.println("ms");
            Serial.println("==============================");
        }
        
        Serial.print("RFID Tag ID:"); // print a header to the Serial port.
        // loop through the byte array
        for(int n = 0; n < 5; n++) {
            Serial.print(tagData[n], DEC);  // print the byte in Decimal format
            if(n < 4) { // only print the comma on the first 4 numbers
                Serial.print(",");
            }
        }
        Serial.print("\n\r"); // return character for next line
        
        // Handle RFID pairing mode
        if (rfidPairingMode) {
            if (addRfidKey(tagData)) {
                Serial.println("RFID: Key paired successfully!");
                rfidPairingMode = false;
            } else {
                Serial.println("RFID: Failed to pair key (already exists or storage full)");
            }
        }
        // Handle RFID authentication
        else if (checkRfidKey(tagData)) {
            rfidAuthenticated = true;
            rfidAuthStartTime = millis();
            Serial.println("RFID: Authenticated for 30 seconds");
        }
    }
    
    // Check RFID authentication timeout (only when Ghost Key is enabled)
    if (rfidAuthenticated && (millis() - rfidAuthStartTime >= RFID_AUTH_TIMEOUT)) {
        rfidAuthenticated = false;
        Serial.println("RFID: Authentication expired");
    }
} else {
    // Ghost Key disabled: Reset RFID authentication state
    rfidAuthenticated = false;
    }
    
    // Check for RSSI scan timeout (if no results after 10 seconds)
    static bool rssiTimeoutChecked = false;
    if (!firstRSSIScanDone && !rssiTimeoutChecked && firstRSSIScanStartTime > 0) {
        if (millis() - firstRSSIScanStartTime >= 10000) { // 10 second timeout
            rssiTimeoutChecked = true;
            Serial.println("=== RSSI SCAN TIMEOUT ===");
            Serial.print("No RSSI scan results received after: ");
            Serial.print(millis() - firstRSSIScanStartTime);
            Serial.println("ms");
            Serial.print("Total time from boot: ");
            Serial.print(millis() - systemBootTime);
            Serial.println("ms");
            Serial.println("This may indicate no Bluetooth devices are advertising nearby");
            Serial.println("=========================");
        }
    }
    
    // Check for RFID scan timeout (if no reads after 15 seconds)
    static bool rfidTimeoutChecked = false;
    if (!firstRfidReadDone && !rfidTimeoutChecked && firstRfidScanStartTime > 0) {
        if (millis() - firstRfidScanStartTime >= 15000) { // 15 second timeout
            rfidTimeoutChecked = true;
            Serial.println("=== RFID SCAN TIMEOUT ===");
            Serial.print("No RFID tag read after: ");
            Serial.print(millis() - firstRfidScanStartTime);
            Serial.println("ms");
            Serial.print("Total time from boot: ");
            Serial.print(millis() - systemBootTime);
            Serial.println("ms");
            Serial.print("Time from RFID init: ");
            Serial.print(millis() - rfidInitCompleteTime);
            Serial.println("ms");
            Serial.println("This may indicate no RFID tag is present near the reader");
            Serial.println("========================");
        }
    }
    
    // Status printing
    static unsigned long lastStatusPrint = 0;
    if (millis() - lastStatusPrint >= STATUS_PRINT_INTERVAL) {
        printSystemStatus();
        lastStatusPrint = millis();
    }
}


// setupPins - Initialize all GPIO pins for inputs and outputs
// Called from: setup() function
// Links to: All pin constants defined at top of file
void setupPins() {
    // Input pins (active low with pullups)
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BRAKE_PIN, INPUT_PULLUP);
    pinMode(ACCESSORY_INPUT_PIN, INPUT); // External pullup, no internal pullup needed
    
    // Output pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_LED_PIN, OUTPUT);
    pinMode(BEEPER_PIN, OUTPUT);
    
    // RFID pins - start timing
    rfidInitStartTime = millis();
    
    pinMode(RFID_MOD, OUTPUT);
    pinMode(RFID_SHD, OUTPUT);
    pinMode(RFID_DEMOD_OUT, INPUT);
    pinMode(RFID_RDY_CLK, INPUT);
    
    // Set RFID pins to initial state
    digitalWrite(RFID_SHD, LOW);
    digitalWrite(RFID_MOD, LOW);
    
    rfidInitCompleteTime = millis();
    DEBUG_PRINT("RFID pins initialized in: ");
    DEBUG_PRINT(rfidInitCompleteTime - rfidInitStartTime);
    DEBUG_PRINTLN("ms");
    
    // PWM for button LED
    ledcSetup(LED_PWM_CHANNEL, LED_PWM_FREQ, LED_PWM_RESOLUTION);
    ledcAttachPin(BUTTON_LED_PIN, LED_PWM_CHANNEL);
    
    // Relay control pins
    pinMode(RELAY_ACCESSORY, OUTPUT);
    pinMode(RELAY_IGNITION1, OUTPUT);
    pinMode(RELAY_IGNITION2, OUTPUT);
    pinMode(RELAY_START, OUTPUT);
    pinMode(RELAY_SECURITY, OUTPUT);
    
    // Start with everything off
    digitalWrite(LED_PIN, LOW);
    ledcWrite(LED_PWM_CHANNEL, 0);
    
    digitalWrite(RELAY_ACCESSORY, LOW);
    digitalWrite(RELAY_IGNITION1, LOW);
    digitalWrite(RELAY_IGNITION2, LOW);
    digitalWrite(RELAY_START, LOW);
    digitalWrite(RELAY_SECURITY, LOW);
}



// handleConfigModeOnly - Only handle config mode button press (for Ghost Power only)
// Called from: main loop() when Ghost Key is disabled but Ghost Power is enabled
// Links to: Allows config mode access without push-to-start functionality
void handleConfigModeOnly() {
    bool buttonReading = digitalRead(BUTTON_PIN);
    bool brakeReading = digitalRead(BRAKE_PIN);

    // Handle button press detection
    if (buttonReading == LOW && !buttonPressed) {  // Button just pressed
        buttonPressed = true;
        buttonPressStartTime = millis();
        isLongPressDetected = false;
        DEBUG_BUTTON_PRINTLN("Button pressed (config mode only)");
    } 
    else if (buttonReading == HIGH && buttonPressed) {  // Button just released
        buttonPressed = false;
        DEBUG_BUTTON_PRINTLN("Button released (config mode only)");
        
        if (isLongPressDetected) {
            isLongPressDetected = false;
        }
    }

    // Check for long press without brake - config mode can be accessed
    if (buttonPressed && !brakeHeld && !isLongPressDetected) {
        unsigned long pressDuration = millis() - buttonPressStartTime;
        if (pressDuration >= CONFIG_MODE_PRESS_TIME) {
            isLongPressDetected = true;
            DEBUG_BUTTON_PRINTLN("Long press detected - Entering config mode (Ghost Power only)");
            enterConfigMode();
        }
    }

    // Check for start button press in config mode with pairing active
    if (currentState == CONFIG_MODE && isPairingMode && 
        buttonReading == LOW && lastButtonReading == HIGH && 
        (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
        DEBUG_BUTTON_PRINTLN("Start button pressed in pairing mode - Exiting config mode");
        exitConfigMode();
        lastButtonPress = millis();
    }

    // Update last states
    lastButtonReading = buttonReading;
    lastBrakeReading = brakeReading;
}

void handleButtonPress() {
    bool buttonReading = digitalRead(BUTTON_PIN);
    bool brakeReading = digitalRead(BRAKE_PIN);

    // Update brake held state and LED
    if (brakeReading == LOW && !brakeHeld) {
        brakeHeld = true;
        digitalWrite(BUTTON_LED_PIN, HIGH);
        DEBUG_BUTTON_PRINTLN("Brake held");
    } else if (brakeReading == HIGH && brakeHeld) {
        brakeHeld = false;
        digitalWrite(BUTTON_LED_PIN, LOW);
        DEBUG_BUTTON_PRINTLN("Brake released");
    }

    // Handle button press detection
    if (buttonReading == LOW && !buttonPressed) {  // Button just pressed
        buttonPressed = true;
        buttonPressStartTime = millis();
        isLongPressDetected = false;
        DEBUG_BUTTON_PRINTLN("Button pressed");
    } 
    else if (buttonReading == HIGH && buttonPressed) {  // Button just released
        buttonPressed = false;
        DEBUG_BUTTON_PRINTLN("Button released");
        
        // If we were in a long press but didn't trigger config mode, reset
        if (isLongPressDetected) {
            isLongPressDetected = false;
        }
    }

    // Check for long press without brake - config mode can be accessed regardless of security state
    if (buttonPressed && !brakeHeld && !isLongPressDetected) {
        unsigned long pressDuration = millis() - buttonPressStartTime;
        if (pressDuration >= CONFIG_MODE_PRESS_TIME) {
            isLongPressDetected = true;
            DEBUG_BUTTON_PRINTLN("Long press detected - Entering config mode (security override)");
            enterConfigMode();
        }
    }

    // Check for start button press in config mode with pairing active
    if (currentState == CONFIG_MODE && isPairingMode && 
        buttonReading == LOW && lastButtonReading == HIGH && 
        (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
        DEBUG_BUTTON_PRINTLN("Start button pressed in pairing mode - Exiting config mode");
        exitConfigMode();
        lastButtonPress = millis();
    }
    
    // Only process normal button operations if not in config mode
    if (currentState != CONFIG_MODE) {
        // Check for button press while brake is held
        if (buttonReading == LOW && brakeHeld && 
            (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
            lastButtonPress = millis();
            
            // Check if authenticated by Bluetooth (if enabled) or RFID
            bool isAuthenticated = rfidAuthenticated || (bluetoothEnabled && bluetoothAuthenticated);
            if (isAuthenticated) {
                if (engineRunning) {
                    // If engine is running, only allow turning off
                    DEBUG_PRINTLN("Engine sequence stopped");
                    engineRunning = false;
                    systemState = 0;  // Set to OFF
                    startRelayActive = false;
                    lastEngineShutdown = millis();  // Record time of engine shutdown
                    controlRelays();
                } else if (!startRelayActive) {
                    // Only allow starting sequence if engine is not running and not already starting
                    DEBUG_PRINTLN("Starting engine sequence...");
                    startRelayActive = true;
                    startRelayTimer = millis();
                    controlRelays();
                }
            } else {
                DEBUG_BUTTON_PRINTLN("Not authenticated (Bluetooth) - Access denied");
                // Error feedback
                for (int i = 0; i < 3; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(50);
                    digitalWrite(LED_PIN, LOW);
                    delay(50);
                }
            }
        }

        // Check for button press without brake
        if (buttonReading == HIGH && lastButtonReading == LOW && 
            brakeReading == HIGH && (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
            // Only process button release if we're not in shutdown delay
            if (!isShuttingDown && !engineRunning && !startRelayActive) {
                // Check if authenticated by Bluetooth (if enabled) or RFID
                bool isAuthenticated = rfidAuthenticated || (bluetoothEnabled && bluetoothAuthenticated);
                if (isAuthenticated) {  // Only allow normal sequence if engine isn't running
                    systemState = (systemState + 1) % 3;
                    DEBUG_BUTTON_PRINT("Button released. New state: ");
                    DEBUG_BUTTON_PRINTLN(systemState);
                    controlRelays();
                }
            }
            lastButtonPress = millis();
        }
    }

    // Update last states
    lastButtonReading = buttonReading;
    lastBrakeReading = brakeReading;
}

void handleBrakeInput() {
    //brake is handled in handleButtonPress()
}

//function to handle button state changes
void updateButtonState(ButtonState &state, int pin) {
    // Inverting the reading since inputs are active-low
    bool reading = !digitalRead(pin);
    
    // If the button state has changed, start/restart the debounce timer
    if (reading != state.lastState) {
        state.lastDebounceTime = millis();
        DEBUG_BUTTON_PRINT("Button raw state changed to: ");
        DEBUG_BUTTON_PRINTLN(reading ? "HIGH" : "LOW");
    }
    
    // Only update the state if the reading has been stable for the debounce period
    if ((millis() - state.lastDebounceTime) > DEBOUNCE_DELAY) {
        // Only process if the reading is different from the current state
        if (reading != state.currentState) {
            state.currentState = reading;
            state.lastState = reading;
            
            DEBUG_BUTTON_PRINT("Button state stabilized to: ");
            DEBUG_BUTTON_PRINTLN(reading ? "HIGH" : "LOW");
        }
    }
}

void updateSystemState() {
    // Handle start sequence timing
    if (startRelayActive) {
        if (millis() - startRelayTimer >= starterPulseTime) {
            DEBUG_PRINTLN("Start sequence complete - transitioning to running state");
            startRelayActive = false;
            engineRunning = true;
            systemState = 2;  // Set to IGNITION state when engine is running
            controlRelays();
        }
    }

    // Handle LED pulsing in config mode
    if (currentState == CONFIG_MODE) {
        static unsigned long lastLedUpdate = 0;
        if (millis() - lastLedUpdate > 20) {  // Update every 20ms for smooth fade
            ledBrightness = ledBrightness + ledFadeAmount;
            
            // Reverse the direction of the fading at the ends of the fade
            if (ledBrightness <= 0 || ledBrightness >= 255) {
                ledFadeAmount = -ledFadeAmount;
            }
            
            ledcWrite(LED_PWM_CHANNEL, ledBrightness);
            lastLedUpdate = millis();
        }
    } else {
        // Normal LED control when not in config mode
        if (brakeHeld) {
            ledcWrite(LED_PWM_CHANNEL, 255);  // Full brightness when brake is held
        } else {
            ledcWrite(LED_PWM_CHANNEL, 0);    // Off when brake is not held
        }
    }
}

void checkStartSequence() {
    if (startRelayActive) {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - startRelayTimer;
        DEBUG_PRINT("Start sequence time: ");
        DEBUG_PRINT(elapsedTime);
        DEBUG_PRINT(" / ");
        DEBUG_PRINTLN(starterPulseTime);
        
        if (elapsedTime >= starterPulseTime) {
            DEBUG_PRINTLN("Start sequence complete - transitioning to running state");
            startRelayActive = false;
            engineRunning = true;
            systemState = 2;  // Set to IGNITION state when engine is running
            controlRelays();
        }
    }
}

// controlRelays - Manage car ignition relays based on system state
// Called from: main loop() continuously
// Links to: Uses systemState, engineRunning, startRelayActive variables
void controlRelays() {
    if (engineRunning) {
        systemState = 2;
    }

    // Track states to avoid redundant GPIO writes (CPU optimization)
    static bool lastAccessory = false, lastIgnition1 = false, lastIgnition2 = false, lastStart = false;
    bool newAccessory, newIgnition1, newIgnition2, newStart;

    // Determine relay states
    if (engineRunning) {
        // Engine running - all on except starter
        newAccessory = HIGH; newIgnition1 = HIGH; newIgnition2 = HIGH; newStart = LOW;
    } else if (startRelayActive) {
        // Starting sequence - only IGN2 and START
        newAccessory = LOW; newIgnition1 = LOW; newIgnition2 = HIGH; newStart = HIGH;
    } else {
        // Normal button sequence
        switch (systemState) {
            case 0:  // Off
                newAccessory = LOW; newIgnition1 = LOW; newIgnition2 = LOW; newStart = LOW;
                break;
            case 1:  // Accessory
                newAccessory = HIGH; newIgnition1 = LOW; newIgnition2 = LOW; newStart = LOW;
                break;
            case 2:  // Ignition
                newAccessory = HIGH; newIgnition1 = HIGH; newIgnition2 = HIGH; newStart = LOW;
                break;
            default:
                newAccessory = LOW; newIgnition1 = LOW; newIgnition2 = LOW; newStart = LOW;
                break;
        }
    }
    
    // Only update GPIO if changed
    if (newAccessory != lastAccessory) { digitalWrite(RELAY_ACCESSORY, newAccessory); lastAccessory = newAccessory; }
    if (newIgnition1 != lastIgnition1) { digitalWrite(RELAY_IGNITION1, newIgnition1); lastIgnition1 = newIgnition1; }
    if (newIgnition2 != lastIgnition2) { digitalWrite(RELAY_IGNITION2, newIgnition2); lastIgnition2 = newIgnition2; }
    if (newStart != lastStart) { digitalWrite(RELAY_START, newStart); lastStart = newStart; }
}

// call this when starting the vehicle
void triggerStartRelayPulse() {
    startRelayPulseStart = millis();
    startRelayPulsing = true;
}

void checkAutoLock() {
    // TBD
}

void enterConfigMode() {
    currentState = CONFIG_MODE;
    DEBUG_PRINTLN("Entering configuration mode");
    
    // Initialize WiFi and web server
    setupWiFi();
    
    // Initialize mDNS
    if (!MDNS.begin("ghost-key")) {
        Serial.println("Error setting up MDNS responder!");
    } else {
        Serial.println("mDNS responder started - access via http://ghost-key.local");
    }
    
    setupWebServer();
    
    // Visual feedback
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
    
    // Start LED pulsing
    ledBrightness = 0;
    ledFadeAmount = 5;
}

void exitConfigMode() {
    currentState = OFF;
    DEBUG_PRINTLN("Exiting configuration mode");
    
    // Disable pairing mode when exiting config
    disablePairingMode();
    
    // Apply any WiFi password changes before stopping WiFi
    if (wifiEnabled) {
        WiFi.softAPdisconnect(true);
        wifiEnabled = false;
        
        // Check if there's a new WiFi password and restart with it
        String savedPassword = preferences.getString("wifi_password", "123456789");
        if (savedPassword != ap_password) {
            DEBUG_PRINTLN("Applying WiFi password change on exit");
            ap_password = savedPassword;
        }
    }
    // Note: WebServer doesn't have an end() method, server stops when WiFi stops
    
    // Visual feedback
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    
    // Turn off LED
    ledcWrite(LED_PWM_CHANNEL, 0);
}

void setupWiFi() {
    DEBUG_PRINTLN("Starting WiFi Access Point...");
    WiFi.disconnect();  // Disconnect from any existing connections
    WiFi.mode(WIFI_AP);  // Set WiFi to AP mode
    WiFi.softAP(ap_ssid, ap_password.c_str());
    
    IPAddress IP = WiFi.softAPIP();
    DEBUG_PRINT("AP IP address: ");
    DEBUG_PRINTLN(IP);
    DEBUG_PRINT("WiFi password: ");
    DEBUG_PRINTLN(ap_password);
    
    wifiEnabled = true;
}

// Function to get device info as JSON with caching (optimized for memory)
const char* getDevicesJson() {
    int bondedCount = esp_ble_get_bond_device_num();
    
    // Check if we can use cached JSON
    if (jsonCacheValid && 
        lastDeviceCount == bondedCount && 
        (millis() - lastJsonUpdate < 15000)) {  // Extended cache to 15 seconds
        return cachedDevicesJson;
    }
    
    // Build JSON directly into buffer to avoid heap fragmentation
    char* jsonPtr = cachedDevicesJson;
    int remaining = sizeof(cachedDevicesJson) - 1;
    
    // Start JSON array
    int written = snprintf(jsonPtr, remaining, "[");
    jsonPtr += written; remaining -= written;
    
    if (bondedCount == 0) {
        snprintf(jsonPtr, remaining, "]");
        jsonCacheValid = true;
        lastDeviceCount = bondedCount;
        lastJsonUpdate = millis();
        return cachedDevicesJson;
    }
    
    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
    if (!bondedDevices) {
        snprintf(jsonPtr, remaining, "]");
        return cachedDevicesJson;
    }
    
    for (int i = 0; i < bondedCount && remaining > 100; i++) {
        char mac[18];
        snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2],
                bondedDevices[i].bd_addr[3], bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
        
        char name[32] = "Unknown Device";
        getDeviceName(bondedDevices[i].bd_addr, name, sizeof(name));
        
        int8_t rssi = -99;
        float distance = 0.0;
        
        if (getDeviceRSSI(bondedDevices[i].bd_addr, &rssi)) {
            distance = calculateDistance(rssi);
        }
        
        // Build JSON object directly
        written = snprintf(jsonPtr, remaining, 
            "%s{\"mac\":\"%s\",\"name\":\"%s\",\"priority\":false,\"rssi\":%d,\"distance\":%.1f}",
            (i > 0) ? "," : "", mac, name, rssi, distance);
        
        if (written > 0 && written < remaining) {
            jsonPtr += written; 
            remaining -= written;
        } else {
            break; // Buffer full
        }
    }
    
    // Close JSON array
    snprintf(jsonPtr, remaining, "]");
    
    releaseBondedDevicesBuffer(bondedDevices);
    
    // Cache the result
    jsonCacheValid = true;
    lastDeviceCount = bondedCount;
    lastJsonUpdate = millis();
    
    return cachedDevicesJson;
}

void setupWebServer() {
    Serial.println("Starting Web Server...");

    // Handle root path - serve setup page if first time, otherwise normal page
    server.on("/", HTTP_GET, [](){
        if (!firstSetupComplete) {
            Serial.println("Serving first-time setup page");
            server.send_P(200, "text/html", setup_html);
        } else {
        Serial.println("Serving main configuration page");
        server.send_P(200, "text/html", config_html);
        }
    });
    
    // Handle logo requests - serve JDI SVG from PROGMEM
    server.on("/logo", HTTP_GET, [](){
        Serial.println("Logo request received - serving JDI logo from PROGMEM");
        server.send_P(200, "image/svg+xml", jdi_logo_svg);
    });
    
    // Handle icon requests for PWA - serve simple PNG icon
    server.on("/icon.png", HTTP_GET, [](){
        Serial.println("PWA icon request received - serving PNG icon");
        // Simple 192x192 PNG icon (minimal size for compatibility)
        // This creates a simple filled circle as a placeholder
        static const uint8_t pwa_icon_png[] PROGMEM = {
            0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 
  0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x08, 0x02, 0x00, 0x00, 0x00, 0xdd, 0xbe, 0xfb, 
  0x50, 0x00, 0x00, 0x0a, 0x49, 0x69, 0x43, 0x43, 0x50, 0x73, 0x52, 0x47, 0x42, 0x20, 0x49, 0x45, 
  0x43, 0x36, 0x31, 0x39, 0x36, 0x36, 0x2d, 0x32, 0x2e, 0x31, 0x00, 0x00, 0x48, 0x89, 0x9d, 0x53, 
  0x77, 0x58, 0x93, 0xf7, 0x16, 0x3e, 0xdf, 0xf7, 0x65, 0x0f, 0x56, 0x42, 0xd8, 0xf0, 0xb1, 0x97, 
  0x6c, 0x81, 0x00, 0x22, 0x23, 0xac, 0x08, 0xc8, 0x10, 0x59, 0xa2, 0x10, 0x92, 0x00, 0x61, 0x84, 
  0x10, 0x12, 0x40, 0xc5, 0x85, 0x88, 0x0a, 0x56, 0x14, 0x15, 0x11, 0x9c, 0x48, 0x55, 0xc4, 0x82, 
  0xd5, 0x0a, 0x48, 0x9d, 0x88, 0xe2, 0xa0, 0x28, 0xb8, 0x67, 0x41, 0x8a, 0x88, 0x5a, 0x8b, 0x55, 
  0x5c, 0x38, 0xee, 0x1f, 0xdc, 0xa7, 0xb5, 0x7d, 0x7a, 0xef, 0xed, 0xed, 0xfb, 0xd7, 0xfb, 0xbc, 
  0xe7, 0x9c, 0xe7, 0xfc, 0xce, 0x79, 0xcf, 0x0f, 0x80, 0x11, 0x12, 0x26, 0x91, 0xe6, 0xa2, 0x6a, 
  0x00, 0x39, 0x52, 0x85, 0x3c, 0x3a, 0xd8, 0x1f, 0x8f, 0x4f, 0x48, 0xc4, 0xc9, 0xbd, 0x80, 0x02, 
  0x15, 0x48, 0xe0, 0x04, 0x20, 0x10, 0xe6, 0xcb, 0xc2, 0x67, 0x05, 0xc5, 0x00, 0x00, 0xf0, 0x03, 
  0x79, 0x78, 0x7e, 0x74, 0xb0, 0x3f, 0xfc, 0x01, 0xaf, 0x6f, 0x00, 0x02, 0x00, 0x70, 0xd5, 0x2e, 
  0x24, 0x12, 0xc7, 0xe1, 0xff, 0x83, 0xba, 0x50, 0x26, 0x57, 0x00, 0x20, 0x91, 0x00, 0xe0, 0x22, 
  0x12, 0xe7, 0x0b, 0x01, 0x90, 0x52, 0x00, 0xc8, 0x2e, 0x54, 0xc8, 0x14, 0x00, 0xc8, 0x18, 0x00, 
  0xb0, 0x53, 0xb3, 0x64, 0x0a, 0x00, 0x94, 0x00, 0x00, 0x6c, 0x79, 0x7c, 0x42, 0x22, 0x00, 0xaa, 
  0x0d, 0x00, 0xec, 0xf4, 0x49, 0x3e, 0x05, 0x00, 0xd8, 0xa9, 0x93, 0xdc, 0x17, 0x00, 0xd8, 0xa2, 
  0x1c, 0xa9, 0x08, 0x00, 0x8d, 0x01, 0x00, 0x99, 0x28, 0x47, 0x24, 0x02, 0x40, 0xbb, 0x00, 0x60, 
  0x55, 0x81, 0x52, 0x2c, 0x02, 0xc0, 0xc2, 0x00, 0xa0, 0xac, 0x40, 0x22, 0x2e, 0x04, 0xc0, 0xae, 
  0x01, 0x80, 0x59, 0xb6, 0x32, 0x47, 0x02, 0x80, 0xbd, 0x05, 0x00, 0x76, 0x8e, 0x58, 0x90, 0x0f, 
  0x40, 0x60, 0x00, 0x80, 0x99, 0x42, 0x2c, 0xcc, 0x00, 0x20, 0x38, 0x02, 0x00, 0x43, 0x1e, 0x13, 
  0xcd, 0x03, 0x20, 0x4c, 0x03, 0xa0, 0x30, 0xd2, 0xbf, 0xe0, 0xa9, 0x5f, 0x70, 0x85, 0xb8, 0x48, 
  0x01, 0x00, 0xc0, 0xcb, 0x95, 0xcd, 0x97, 0x4b, 0xd2, 0x33, 0x14, 0xb8, 0x95, 0xd0, 0x1a, 0x77, 
  0xf2, 0xf0, 0xe0, 0xe2, 0x21, 0xe2, 0xc2, 0x6c, 0xb1, 0x42, 0x61, 0x17, 0x29, 0x10, 0x66, 0x09, 
  0xe4, 0x22, 0x9c, 0x97, 0x9b, 0x23, 0x13, 0x48, 0xe7, 0x03, 0x4c, 0xce, 0x0c, 0x00, 0x00, 0x1a, 
  0xf9, 0xd1, 0xc1, 0xfe, 0x38, 0x3f, 0x90, 0xe7, 0xe6, 0xe4, 0xe1, 0xe6, 0x66, 0xe7, 0x6c, 0xef, 
  0xf4, 0xc5, 0xa2, 0xfe, 0x6b, 0xf0, 0x6f, 0x22, 0x3e, 0x21, 0xf1, 0xdf, 0xfe, 0xbc, 0x8c, 0x02, 
  0x04, 0x00, 0x10, 0x4e, 0xcf, 0xef, 0xda, 0x5f, 0xe5, 0xe5, 0xd6, 0x03, 0x70, 0xc7, 0x01, 0xb0, 
  0x75, 0xbf, 0x6b, 0xa9, 0x5b, 0x00, 0xda, 0x56, 0x00, 0x68, 0xdf, 0xf9, 0x5d, 0x33, 0xdb, 0x09, 
  0xa0, 0x5a, 0x0a, 0xd0, 0x7a, 0xf9, 0x8b, 0x79, 0x38, 0xfc, 0x40, 0x1e, 0x9e, 0xa1, 0x50, 0xc8, 
  0x3c, 0x1d, 0x1c, 0x0a, 0x0b, 0x0b, 0xed, 0x25, 0x62, 0xa1, 0xbd, 0x30, 0xe3, 0x8b, 0x3e, 0xff, 
  0x33, 0xe1, 0x6f, 0xe0, 0x8b, 0x7e, 0xf6, 0xfc, 0x40, 0x1e, 0xfe, 0xdb, 0x7a, 0xf0, 0x00, 0x71, 
  0x9a, 0x40, 0x99, 0xad, 0xc0, 0xa3, 0x83, 0xfd, 0x71, 0x61, 0x6e, 0x76, 0xae, 0x52, 0x8e, 0xe7, 
  0xcb, 0x04, 0x42, 0x31, 0x6e, 0xf7, 0xe7, 0x23, 0xfe, 0xc7, 0x85, 0x7f, 0xfd, 0x8e, 0x29, 0xd1, 
  0xe2, 0x34, 0xb1, 0x5c, 0x2c, 0x15, 0x8a, 0xf1, 0x58, 0x89, 0xb8, 0x50, 0x22, 0x4d, 0xc7, 0x79, 
  0xb9, 0x52, 0x91, 0x44, 0x21, 0xc9, 0x95, 0xe2, 0x12, 0xe9, 0x7f, 0x32, 0xf1, 0x1f, 0x96, 0xfd, 
  0x09, 0x93, 0x77, 0x0d, 0x00, 0xac, 0x86, 0x4f, 0xc0, 0x4e, 0xb6, 0x07, 0xb5, 0xcb, 0x6c, 0xc0, 
  0x7e, 0xee, 0x01, 0x02, 0x8b, 0x0e, 0x58, 0xd2, 0x76, 0x00, 0x40, 0x7e, 0xf3, 0x2d, 0x8c, 0x1a, 
  0x0b, 0x91, 0x00, 0x10, 0x67, 0x34, 0x32, 0x79, 0xf7, 0x00, 0x00, 0x93, 0xbf, 0xf9, 0x8f, 0x40, 
  0x2b, 0x01, 0x00, 0xcd, 0x97, 0xa4, 0xe3, 0x00, 0x00, 0xbc, 0xe8, 0x18, 0x5c, 0xa8, 0x94, 0x17, 
  0x4c, 0xc6, 0x08, 0x00, 0x00, 0x44, 0xa0, 0x81, 0x2a, 0xb0, 0x41, 0x07, 0x0c, 0xc1, 0x14, 0xac, 
  0xc0, 0x0e, 0x9c, 0xc1, 0x1d, 0xbc, 0xc0, 0x17, 0x02, 0x61, 0x06, 0x44, 0x40, 0x0c, 0x24, 0xc0, 
  0x3c, 0x10, 0x42, 0x06, 0xe4, 0x80, 0x1c, 0x0a, 0xa1, 0x18, 0x96, 0x41, 0x19, 0x54, 0xc0, 0x3a, 
  0xd8, 0x04, 0xb5, 0xb0, 0x03, 0x1a, 0xa0, 0x11, 0x9a, 0xe1, 0x10, 0xb4, 0xc1, 0x31, 0x38, 0x0d, 
  0xe7, 0xe0, 0x12, 0x5c, 0x81, 0xeb, 0x70, 0x17, 0x06, 0x60, 0x18, 0x9e, 0xc2, 0x18, 0xbc, 0x86, 
  0x09, 0x04, 0x41, 0xc8, 0x08, 0x13, 0x61, 0x21, 0x3a, 0x88, 0x11, 0x62, 0x8e, 0xd8, 0x22, 0xce, 
  0x08, 0x17, 0x99, 0x8e, 0x04, 0x22, 0x61, 0x48, 0x34, 0x92, 0x80, 0xa4, 0x20, 0xe9, 0x88, 0x14, 
  0x51, 0x22, 0xc5, 0xc8, 0x72, 0xa4, 0x02, 0xa9, 0x42, 0x6a, 0x91, 0x5d, 0x48, 0x23, 0xf2, 0x2d, 
  0x72, 0x14, 0x39, 0x8d, 0x5c, 0x40, 0xfa, 0x90, 0xdb, 0xc8, 0x20, 0x32, 0x8a, 0xfc, 0x8a, 0xbc, 
  0x47, 0x31, 0x94, 0x81, 0xb2, 0x51, 0x03, 0xd4, 0x02, 0x75, 0x40, 0xb9, 0xa8, 0x1f, 0x1a, 0x8a, 
  0xc6, 0xa0, 0x73, 0xd1, 0x74, 0x34, 0x0f, 0x5d, 0x80, 0x96, 0xa2, 0x6b, 0xd1, 0x1a, 0xb4, 0x1e, 
  0x3d, 0x80, 0xb6, 0xa2, 0xa7, 0xd1, 0x4b, 0xe8, 0x75, 0x74, 0x00, 0x7d, 0x8a, 0x8e, 0x63, 0x80, 
  0xd1, 0x31, 0x0e, 0x66, 0x8c, 0xd9, 0x61, 0x5c, 0x8c, 0x87, 0x45, 0x60, 0x89, 0x58, 0x1a, 0x26, 
  0xc7, 0x16, 0x63, 0xe5, 0x58, 0x35, 0x56, 0x8f, 0x35, 0x63, 0x1d, 0x58, 0x37, 0x76, 0x15, 0x1b, 
  0xc0, 0x9e, 0x61, 0xef, 0x08, 0x24, 0x02, 0x8b, 0x80, 0x13, 0xec, 0x08, 0x5e, 0x84, 0x10, 0xc2, 
  0x6c, 0x82, 0x90, 0x90, 0x47, 0x58, 0x4c, 0x58, 0x43, 0xa8, 0x25, 0xec, 0x23, 0xb4, 0x12, 0xba, 
  0x08, 0x57, 0x09, 0x83, 0x84, 0x31, 0xc2, 0x27, 0x22, 0x93, 0xa8, 0x4f, 0xb4, 0x25, 0x7a, 0x12, 
  0xf9, 0xc4, 0x78, 0x62, 0x3a, 0xb1, 0x90, 0x58, 0x46, 0xac, 0x26, 0xee, 0x21, 0x1e, 0x21, 0x9e, 
  0x25, 0x5e, 0x27, 0x0e, 0x13, 0x5f, 0x93, 0x48, 0x24, 0x0e, 0xc9, 0x92, 0xe4, 0x4e, 0x0a, 0x21, 
  0x25, 0x90, 0x32, 0x49, 0x0b, 0x49, 0x6b, 0x48, 0xdb, 0x48, 0x2d, 0xa4, 0x53, 0xa4, 0x3e, 0xd2, 
  0x10, 0x69, 0x9c, 0x4c, 0x26, 0xeb, 0x90, 0x6d, 0xc9, 0xde, 0xe4, 0x08, 0xb2, 0x80, 0xac, 0x20, 
  0x97, 0x91, 0xb7, 0x90, 0x0f, 0x90, 0x4f, 0x92, 0xfb, 0xc9, 0xc3, 0xe4, 0xb7, 0x14, 0x3a, 0xc5, 
  0x88, 0xe2, 0x4c, 0x09, 0xa2, 0x24, 0x52, 0xa4, 0x94, 0x12, 0x4a, 0x35, 0x65, 0x3f, 0xe5, 0x04, 
  0xa5, 0x9f, 0x32, 0x42, 0x99, 0xa0, 0xaa, 0x51, 0xcd, 0xa9, 0x9e, 0xd4, 0x08, 0xaa, 0x88, 0x3a, 
  0x9f, 0x5a, 0x49, 0x6d, 0xa0, 0x76, 0x50, 0x2f, 0x53, 0x87, 0xa9, 0x13, 0x34, 0x75, 0x9a, 0x25, 
  0xcd, 0x9b, 0x16, 0x43, 0xcb, 0xa4, 0x2d, 0xa3, 0xd5, 0xd0, 0x9a, 0x69, 0x67, 0x69, 0xf7, 0x68, 
  0x2f, 0xe9, 0x74, 0xba, 0x09, 0xdd, 0x83, 0x1e, 0x45, 0x97, 0xd0, 0x97, 0xd2, 0x6b, 0xe8, 0x07, 
  0xe9, 0xe7, 0xe9, 0x83, 0xf4, 0x77, 0x0c, 0x0d, 0x86, 0x0d, 0x83, 0xc7, 0x48, 0x62, 0x28, 0x19, 
  0x6b, 0x19, 0x7b, 0x19, 0xa7, 0x18, 0xb7, 0x19, 0x2f, 0x99, 0x4c, 0xa6, 0x05, 0xd3, 0x97, 0x99, 
  0xc8, 0x54, 0x30, 0xd7, 0x32, 0x1b, 0x99, 0x67, 0x98, 0x0f, 0x98, 0x6f, 0x55, 0x58, 0x2a, 0xf6, 
  0x2a, 0x7c, 0x15, 0x91, 0xca, 0x12, 0x95, 0x3a, 0x95, 0x56, 0x95, 0x7e, 0x95, 0xe7, 0xaa, 0x54, 
  0x55, 0x73, 0x55, 0x3f, 0xd5, 0x79, 0xaa, 0x0b, 0x54, 0xab, 0x55, 0x0f, 0xab, 0x5e, 0x56, 0x7d, 
  0xa6, 0x46, 0x55, 0xb3, 0x50, 0xe3, 0xa9, 0x09, 0xd4, 0x16, 0xab, 0xd5, 0xa9, 0x1d, 0x55, 0xbb, 
  0xa9, 0x36, 0xae, 0xce, 0x52, 0x77, 0x52, 0x8f, 0x50, 0xcf, 0x51, 0x5f, 0xa3, 0xbe, 0x5f, 0xfd, 
  0x82, 0xfa, 0x63, 0x0d, 0xb2, 0x86, 0x85, 0x46, 0xa0, 0x86, 0x48, 0xa3, 0x54, 0x63, 0xb7, 0xc6, 
  0x19, 0x8d, 0x21, 0x16, 0xc6, 0x32, 0x65, 0xf1, 0x58, 0x42, 0xd6, 0x72, 0x56, 0x03, 0xeb, 0x2c, 
  0x6b, 0x98, 0x4d, 0x62, 0x5b, 0xb2, 0xf9, 0xec, 0x4c, 0x76, 0x05, 0xfb, 0x1b, 0x76, 0x2f, 0x7b, 
  0x4c, 0x53, 0x43, 0x73, 0xaa, 0x66, 0xac, 0x66, 0x91, 0x66, 0x9d, 0xe6, 0x71, 0xcd, 0x01, 0x0e, 
  0xc6, 0xb1, 0xe0, 0xf0, 0x39, 0xd9, 0x9c, 0x4a, 0xce, 0x21, 0xce, 0x0d, 0xce, 0x7b, 0x2d, 0x03, 
  0x2d, 0x3f, 0x2d, 0xb1, 0xd6, 0x6a, 0xad, 0x66, 0xad, 0x7e, 0xad, 0x37, 0xda, 0x7a, 0xda, 0xbe, 
  0xda, 0x62, 0xed, 0x72, 0xed, 0x16, 0xed, 0xeb, 0xda, 0xef, 0x75, 0x70, 0x9d, 0x40, 0x9d, 0x2c, 
  0x9d, 0xf5, 0x3a, 0x6d, 0x3a, 0xf7, 0x75, 0x09, 0xba, 0x36, 0xba, 0x51, 0xba, 0x85, 0xba, 0xdb, 
  0x75, 0xcf, 0xea, 0x3e, 0xd3, 0x63, 0xeb, 0x79, 0xe9, 0x09, 0xf5, 0xca, 0xf5, 0x0e, 0xe9, 0xdd, 
  0xd1, 0x47, 0xf5, 0x6d, 0xf4, 0xa3, 0xf5, 0x17, 0xea, 0xef, 0xd6, 0xef, 0xd1, 0x1f, 0x37, 0x30, 
  0x34, 0x08, 0x36, 0x90, 0x19, 0x6c, 0x31, 0x38, 0x63, 0xf0, 0xcc, 0x90, 0x63, 0xe8, 0x6b, 0x98, 
  0x69, 0xb8, 0xd1, 0xf0, 0x84, 0xe1, 0xa8, 0x11, 0xcb, 0x68, 0xba, 0x91, 0xc4, 0x68, 0xa3, 0xd1, 
  0x49, 0xa3, 0x27, 0xb8, 0x26, 0xee, 0x87, 0x67, 0xe3, 0x35, 0x78, 0x17, 0x3e, 0x66, 0xac, 0x6f, 
  0x1c, 0x62, 0xac, 0x34, 0xde, 0x65, 0xdc, 0x6b, 0x3c, 0x61, 0x62, 0x69, 0x32, 0xdb, 0xa4, 0xc4, 
  0xa4, 0xc5, 0xe4, 0xbe, 0x29, 0xcd, 0x94, 0x6b, 0x9a, 0x66, 0xba, 0xd1, 0xb4, 0xd3, 0x74, 0xcc, 
  0xcc, 0xc8, 0x2c, 0xdc, 0xac, 0xd8, 0xac, 0xc9, 0xec, 0x8e, 0x39, 0xd5, 0x9c, 0x6b, 0x9e, 0x61, 
  0xbe, 0xd9, 0xbc, 0xdb, 0xfc, 0x8d, 0x85, 0xa5, 0x45, 0x9c, 0xc5, 0x4a, 0x8b, 0x36, 0x8b, 0xc7, 
  0x96, 0xda, 0x96, 0x7c, 0xcb, 0x05, 0x96, 0x4d, 0x96, 0xf7, 0xac, 0x98, 0x56, 0x3e, 0x56, 0x79, 
  0x56, 0xf5, 0x56, 0xd7, 0xac, 0x49, 0xd6, 0x5c, 0xeb, 0x2c, 0xeb, 0x6d, 0xd6, 0x57, 0x6c, 0x50, 
  0x1b, 0x57, 0x9b, 0x0c, 0x9b, 0x3a, 0x9b, 0xcb, 0xb6, 0xa8, 0xad, 0x9b, 0xad, 0xc4, 0x76, 0x9b, 
  0x6d, 0xdf, 0x14, 0xe2, 0x14, 0x8f, 0x29, 0xd2, 0x29, 0xf5, 0x53, 0x6e, 0xda, 0x31, 0xec, 0xfc, 
  0xec, 0x0a, 0xec, 0x9a, 0xec, 0x06, 0xed, 0x39, 0xf6, 0x61, 0xf6, 0x25, 0xf6, 0x6d, 0xf6, 0xcf, 
  0x1d, 0xcc, 0x1c, 0x12, 0x1d, 0xd6, 0x3b, 0x74, 0x3b, 0x7c, 0x72, 0x74, 0x75, 0xcc, 0x76, 0x6c, 
  0x70, 0xbc, 0xeb, 0xa4, 0xe1, 0x34, 0xc3, 0xa9, 0xc4, 0xa9, 0xc3, 0xe9, 0x57, 0x67, 0x1b, 0x67, 
  0xa1, 0x73, 0x9d, 0xf3, 0x35, 0x17, 0xa6, 0x4b, 0x90, 0xcb, 0x12, 0x97, 0x76, 0x97, 0x17, 0x53, 
  0x6d, 0xa7, 0x8a, 0xa7, 0x6e, 0x9f, 0x7a, 0xcb, 0x95, 0xe5, 0x1a, 0xee, 0xba, 0xd2, 0xb5, 0xd3, 
  0xf5, 0xa3, 0x9b, 0xbb, 0x9b, 0xdc, 0xad, 0xd9, 0x6d, 0xd4, 0xdd, 0xcc, 0x3d, 0xc5, 0x7d, 0xab, 
  0xfb, 0x4d, 0x2e, 0x9b, 0x1b, 0xc9, 0x5d, 0xc3, 0x3d, 0xef, 0x41, 0xf4, 0xf0, 0xf7, 0x58, 0xe2, 
  0x71, 0xcc, 0xe3, 0x9d, 0xa7, 0x9b, 0xa7, 0xc2, 0xf3, 0x90, 0xe7, 0x2f, 0x5e, 0x76, 0x5e, 0x59, 
  0x5e, 0xfb, 0xbd, 0x1e, 0x4f, 0xb3, 0x9c, 0x26, 0x9e, 0xd6, 0x30, 0x6d, 0xc8, 0xdb, 0xc4, 0x5b, 
  0xe0, 0xbd, 0xcb, 0x7b, 0x60, 0x3a, 0x3e, 0x3d, 0x65, 0xfa, 0xce, 0xe9, 0x03, 0x3e, 0xc6, 0x3e, 
  0x02, 0x9f, 0x7a, 0x9f, 0x87, 0xbe, 0xa6, 0xbe, 0x22, 0xdf, 0x3d, 0xbe, 0x23, 0x7e, 0xd6, 0x7e, 
  0x99, 0x7e, 0x07, 0xfc, 0x9e, 0xfb, 0x3b, 0xfa, 0xcb, 0xfd, 0x8f, 0xf8, 0xbf, 0xe1, 0x79, 0xf2, 
  0x16, 0xf1, 0x4e, 0x05, 0x60, 0x01, 0xc1, 0x01, 0xe5, 0x01, 0xbd, 0x81, 0x1a, 0x81, 0xb3, 0x03, 
  0x6b, 0x03, 0x1f, 0x04, 0x99, 0x04, 0xa5, 0x07, 0x35, 0x05, 0x8d, 0x05, 0xbb, 0x06, 0x2f, 0x0c, 
  0x3e, 0x15, 0x42, 0x0c, 0x09, 0x0d, 0x59, 0x1f, 0x72, 0x93, 0x6f, 0xc0, 0x17, 0xf2, 0x1b, 0xf9, 
  0x63, 0x33, 0xdc, 0x67, 0x2c, 0x9a, 0xd1, 0x15, 0xca, 0x08, 0x9d, 0x15, 0x5a, 0x1b, 0xfa, 0x30, 
  0xcc, 0x26, 0x4c, 0x1e, 0xd6, 0x11, 0x8e, 0x86, 0xcf, 0x08, 0xdf, 0x10, 0x7e, 0x6f, 0xa6, 0xf9, 
  0x4c, 0xe9, 0xcc, 0xb6, 0x08, 0x88, 0xe0, 0x47, 0x6c, 0x88, 0xb8, 0x1f, 0x69, 0x19, 0x99, 0x17, 
  0xf9, 0x7d, 0x14, 0x29, 0x2a, 0x32, 0xaa, 0x2e, 0xea, 0x51, 0xb4, 0x53, 0x74, 0x71, 0x74, 0xf7, 
  0x2c, 0xd6, 0xac, 0xe4, 0x59, 0xfb, 0x67, 0xbd, 0x8e, 0xf1, 0x8f, 0xa9, 0x8c, 0xb9, 0x3b, 0xdb, 
  0x6a, 0xb6, 0x72, 0x76, 0x67, 0xac, 0x6a, 0x6c, 0x52, 0x6c, 0x63, 0xec, 0x9b, 0xb8, 0x80, 0xb8, 
  0xaa, 0xb8, 0x81, 0x78, 0x87, 0xf8, 0x45, 0xf1, 0x97, 0x12, 0x74, 0x13, 0x24, 0x09, 0xed, 0x89, 
  0xe4, 0xc4, 0xd8, 0xc4, 0x3d, 0x89, 0xe3, 0x73, 0x02, 0xe7, 0x6c, 0x9a, 0x33, 0x9c, 0xe4, 0x9a, 
  0x54, 0x96, 0x74, 0x63, 0xae, 0xe5, 0xdc, 0xa2, 0xb9, 0x17, 0xe6, 0xe9, 0xce, 0xcb, 0x9e, 0x77, 
  0x3c, 0x59, 0x35, 0x59, 0x90, 0x7c, 0x38, 0x85, 0x98, 0x12, 0x97, 0xb2, 0x3f, 0xe5, 0x83, 0x20, 
  0x42, 0x50, 0x2f, 0x18, 0x4f, 0xe5, 0xa7, 0x6e, 0x4d, 0x1d, 0x13, 0xf2, 0x84, 0x9b, 0x85, 0x4f, 
  0x45, 0xbe, 0xa2, 0x8d, 0xa2, 0x51, 0xb1, 0xb7, 0xb8, 0x4a, 0x3c, 0x92, 0xe6, 0x9d, 0x56, 0x95, 
  0xf6, 0x38, 0xdd, 0x3b, 0x7d, 0x43, 0xfa, 0x68, 0x86, 0x4f, 0x46, 0x75, 0xc6, 0x33, 0x09, 0x4f, 
  0x52, 0x2b, 0x79, 0x91, 0x19, 0x92, 0xb9, 0x23, 0xf3, 0x4d, 0x56, 0x44, 0xd6, 0xde, 0xac, 0xcf, 
  0xd9, 0x71, 0xd9, 0x2d, 0x39, 0x94, 0x9c, 0x94, 0x9c, 0xa3, 0x52, 0x0d, 0x69, 0x96, 0xb4, 0x2b, 
  0xd7, 0x30, 0xb7, 0x28, 0xb7, 0x4f, 0x66, 0x2b, 0x2b, 0x93, 0x0d, 0xe4, 0x79, 0xe6, 0x6d, 0xca, 
  0x1b, 0x93, 0x87, 0xca, 0xf7, 0xe4, 0x23, 0xf9, 0x73, 0xf3, 0xdb, 0x15, 0x6c, 0x85, 0x4c, 0xd1, 
  0xa3, 0xb4, 0x52, 0xae, 0x50, 0x0e, 0x16, 0x4c, 0x2f, 0xa8, 0x2b, 0x78, 0x5b, 0x18, 0x5b, 0x78, 
  0xb8, 0x48, 0xbd, 0x48, 0x5a, 0xd4, 0x33, 0xdf, 0x66, 0xfe, 0xea, 0xf9, 0x23, 0x0b, 0x82, 0x16, 
  0x7c, 0xbd, 0x90, 0xb0, 0x50, 0xb8, 0xb0, 0xb3, 0xd8, 0xb8, 0x78, 0x59, 0xf1, 0xe0, 0x22, 0xbf, 
  0x45, 0xbb, 0x16, 0x23, 0x8b, 0x53, 0x17, 0x77, 0x2e, 0x31, 0x5d, 0x52, 0xba, 0x64, 0x78, 0x69, 
  0xf0, 0xd2, 0x7d, 0xcb, 0x68, 0xcb, 0xb2, 0x96, 0xfd, 0x50, 0xe2, 0x58, 0x52, 0x55, 0xf2, 0x6a, 
  0x79, 0xdc, 0xf2, 0x8e, 0x52, 0x83, 0xd2, 0xa5, 0xa5, 0x43, 0x2b, 0x82, 0x57, 0x34, 0x95, 0xa9, 
  0x94, 0xc9, 0xcb, 0x6e, 0xae, 0xf4, 0x5a, 0xb9, 0x63, 0x15, 0x61, 0x95, 0x64, 0x55, 0xef, 0x6a, 
  0x97, 0xd5, 0x5b, 0x56, 0x7f, 0x2a, 0x17, 0x95, 0x5f, 0xac, 0x70, 0xac, 0xa8, 0xae, 0xf8, 0xb0, 
  0x46, 0xb8, 0xe6, 0xe2, 0x57, 0x4e, 0x5f, 0xd5, 0x7c, 0xf5, 0x79, 0x6d, 0xda, 0xda, 0xde, 0x4a, 
  0xb7, 0xca, 0xed, 0xeb, 0x48, 0xeb, 0xa4, 0xeb, 0x6e, 0xac, 0xf7, 0x59, 0xbf, 0xaf, 0x4a, 0xbd, 
  0x6a, 0x41, 0xd5, 0xd0, 0x86, 0xf0, 0x0d, 0xad, 0x1b, 0xf1, 0x8d, 0xe5, 0x1b, 0x5f, 0x6d, 0x4a, 
  0xde, 0x74, 0xa1, 0x7a, 0x6a, 0xf5, 0x8e, 0xcd, 0xb4, 0xcd, 0xca, 0xcd, 0x03, 0x35, 0x61, 0x35, 
  0xed, 0x5b, 0xcc, 0xb6, 0xac, 0xdb, 0xf2, 0xa1, 0x36, 0xa3, 0xf6, 0x7a, 0x9d, 0x7f, 0x5d, 0xcb, 
  0x56, 0xfd, 0xad, 0xab, 0xb7, 0xbe, 0xd9, 0x26, 0xda, 0xd6, 0xbf, 0xdd, 0x77, 0x7b, 0xf3, 0x0e, 
  0x83, 0x1d, 0x15, 0x3b, 0xde, 0xef, 0x94, 0xec, 0xbc, 0xb5, 0x2b, 0x78, 0x57, 0x6b, 0xbd, 0x45, 
  0x7d, 0xf5, 0x6e, 0xd2, 0xee, 0x82, 0xdd, 0x8f, 0x1a, 0x62, 0x1b, 0xba, 0xbf, 0xe6, 0x7e, 0xdd, 
  0xb8, 0x47, 0x77, 0x4f, 0xc5, 0x9e, 0x8f, 0x7b, 0xa5, 0x7b, 0x07, 0xf6, 0x45, 0xef, 0xeb, 0x6a, 
  0x74, 0x6f, 0x6c, 0xdc, 0xaf, 0xbf, 0xbf, 0xb2, 0x09, 0x6d, 0x52, 0x36, 0x8d, 0x1e, 0x48, 0x3a, 
  0x70, 0xe5, 0x9b, 0x80, 0x6f, 0xda, 0x9b, 0xed, 0x9a, 0x77, 0xb5, 0x70, 0x5a, 0x2a, 0x0e, 0xc2, 
  0x41, 0xe5, 0xc1, 0x27, 0xdf, 0xa6, 0x7c, 0x7b, 0xe3, 0x50, 0xe8, 0xa1, 0xce, 0xc3, 0xdc, 0xc3, 
  0xcd, 0xdf, 0x99, 0x7f, 0xb7, 0xf5, 0x08, 0xeb, 0x48, 0x79, 0x2b, 0xd2, 0x3a, 0xbf, 0x75, 0xac, 
  0x2d, 0xa3, 0x6d, 0xa0, 0x3d, 0xa1, 0xbd, 0xef, 0xe8, 0x8c, 0xa3, 0x9d, 0x1d, 0x5e, 0x1d, 0x47, 
  0xbe, 0xb7, 0xff, 0x7e, 0xef, 0x31, 0xe3, 0x63, 0x75, 0xc7, 0x35, 0x8f, 0x57, 0x9e, 0xa0, 0x9d, 
  0x28, 0x3d, 0xf1, 0xf9, 0xe4, 0x82, 0x93, 0xe3, 0xa7, 0x64, 0xa7, 0x9e, 0x9d, 0x4e, 0x3f, 0x3d, 
  0xd4, 0x99, 0xdc, 0x79, 0xf7, 0x4c, 0xfc, 0x99, 0x6b, 0x5d, 0x51, 0x5d, 0xbd, 0x67, 0x43, 0xcf, 
  0x9e, 0x3f, 0x17, 0x74, 0xee, 0x4c, 0xb7, 0x5f, 0xf7, 0xc9, 0xf3, 0xde, 0xe7, 0x8f, 0x5d, 0xf0, 
  0xbc, 0x70, 0xf4, 0x22, 0xf7, 0x62, 0xdb, 0x25, 0xb7, 0x4b, 0xad, 0x3d, 0xae, 0x3d, 0x47, 0x7e, 
  0x70, 0xfd, 0xe1, 0x48, 0xaf, 0x5b, 0x6f, 0xeb, 0x65, 0xf7, 0xcb, 0xed, 0x57, 0x3c, 0xae, 0x74, 
  0xf4, 0x4d, 0xeb, 0x3b, 0xd1, 0xef, 0xd3, 0x7f, 0xfa, 0x6a, 0xc0, 0xd5, 0x73, 0xd7, 0xf8, 0xd7, 
  0x2e, 0x5d, 0x9f, 0x79, 0xbd, 0xef, 0xc6, 0xec, 0x1b, 0xb7, 0x6e, 0x26, 0xdd, 0x1c, 0xb8, 0x25, 
  0xba, 0xf5, 0xf8, 0x76, 0xf6, 0xed, 0x17, 0x77, 0x0a, 0xee, 0x4c, 0xdc, 0x5d, 0x7a, 0x8f, 0x78, 
  0xaf, 0xfc, 0xbe, 0xda, 0xfd, 0xea, 0x07, 0xfa, 0x0f, 0xea, 0x7f, 0xb4, 0xfe, 0xb1, 0x65, 0xc0, 
  0x6d, 0xe0, 0xf8, 0x60, 0xc0, 0x60, 0xcf, 0xc3, 0x59, 0x0f, 0xef, 0x0e, 0x09, 0x87, 0x9e, 0xfe, 
  0x94, 0xff, 0xd3, 0x87, 0xe1, 0xd2, 0x47, 0xcc, 0x47, 0xd5, 0x23, 0x46, 0x23, 0x8d, 0x8f, 0x9d, 
  0x1f, 0x1f, 0x1b, 0x0d, 0x1a, 0xbd, 0xf2, 0x64, 0xce, 0x93, 0xe1, 0xa7, 0xb2, 0xa7, 0x13, 0xcf, 
  0xca, 0x7e, 0x56, 0xff, 0x79, 0xeb, 0x73, 0xab, 0xe7, 0xdf, 0xfd, 0xe2, 0xfb, 0x4b, 0xcf, 0x58, 
  0xfc, 0xd8, 0xf0, 0x0b, 0xf9, 0x8b, 0xcf, 0xbf, 0xae, 0x79, 0xa9, 0xf3, 0x72, 0xef, 0xab, 0xa9, 
  0xaf, 0x3a, 0xc7, 0x23, 0xc7, 0x1f, 0xbc, 0xce, 0x79, 0x3d, 0xf1, 0xa6, 0xfc, 0xad, 0xce, 0xdb, 
  0x7d, 0xef, 0xb8, 0xef, 0xba, 0xdf, 0xc7, 0xbd, 0x1f, 0x99, 0x28, 0xfc, 0x40, 0xfe, 0x50, 0xf3, 
  0xd1, 0xfa, 0x63, 0xc7, 0xa7, 0xd0, 0x4f, 0xf7, 0x3e, 0xe7, 0x7c, 0xfe, 0xfc, 0x2f, 0xf7, 0x84, 
  0xf3, 0xfb, 0x2d, 0x47, 0x38, 0xcf, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 
  0x2e, 0x23, 0x00, 0x00, 0x2e, 0x23, 0x01, 0x78, 0xa5, 0x3f, 0x76, 0x00, 0x00, 0x17, 0xf2, 0x49, 
  0x44, 0x41, 0x54, 0x78, 0x9c, 0xed, 0x9d, 0x79, 0x58, 0x13, 0x47, 0x1f, 0xc7, 0x07, 0x4a, 0xc0, 
  0x80, 0x72, 0xa4, 0x1c, 0x02, 0x15, 0x35, 0x20, 0x82, 0x16, 0x2d, 0xe2, 0x5d, 0xaf, 0xe2, 0x7d, 
  0xe3, 0x7d, 0xd4, 0x5b, 0xab, 0xd5, 0x6a, 0x8f, 0xa7, 0xd6, 0xaa, 0x95, 0xd6, 0x56, 0x8a, 0xd6, 
  0xc7, 0xfb, 0xae, 0x15, 0xb4, 0x16, 0x0f, 0x6a, 0xad, 0x8f, 0xa2, 0xa8, 0xd5, 0x8a, 0x47, 0xb1, 
  0xc5, 0x22, 0x1e, 0x1c, 0x8f, 0x50, 0x04, 0x6a, 0xa8, 0x22, 0x20, 0x88, 0x28, 0x02, 0x21, 0x99, 
  0xf7, 0x8f, 0xf4, 0xc9, 0x9b, 0xe6, 0x58, 0x26, 0xbb, 0x33, 0x99, 0xdd, 0x64, 0x3f, 0x7f, 0x85, 
  0x64, 0x67, 0xe6, 0x3b, 0xb3, 0x5f, 0x66, 0x77, 0x67, 0x67, 0x7e, 0x63, 0x07, 0x21, 0x04, 0x22, 
  0x22, 0x6c, 0xb1, 0xa7, 0x2d, 0x40, 0x44, 0xd8, 0x88, 0x06, 0x12, 0xe1, 0x84, 0x68, 0x20, 0x11, 
  0x4e, 0x88, 0x06, 0x12, 0xe1, 0x84, 0x03, 0x6d, 0x01, 0x02, 0xa3, 0xb6, 0xb6, 0xb6, 0xb4, 0xb4, 
  0xb4, 0xba, 0xba, 0xba, 0xbe, 0xbe, 0x1e, 0x00, 0xe0, 0xe0, 0xe0, 0xe0, 0xe2, 0xe2, 0xe2, 0xe9, 
  0xe9, 0xe9, 0xea, 0xea, 0x4a, 0x5b, 0x1a, 0x1d, 0x44, 0x03, 0x21, 0x91, 0x9a, 0x9a, 0x7a, 0xe1, 
  0xc2, 0x85, 0xd4, 0xd4, 0xd4, 0xac, 0xac, 0xac, 0xb2, 0xb2, 0x32, 0x95, 0x4a, 0xa5, 0xfb, 0xab, 
  0x4c, 0x26, 0x0b, 0x0e, 0x0e, 0xee, 0xde, 0xbd, 0xfb, 0xc0, 0x81, 0x03, 0x87, 0x0d, 0x1b, 0x46, 
  0x4b, 0x24, 0x15, 0xec, 0xc4, 0xc7, 0x78, 0x06, 0x14, 0x0a, 0xc5, 0x77, 0xdf, 0x7d, 0x77, 0xee, 
  0xdc, 0xb9, 0x3f, 0xff, 0xfc, 0x13, 0x31, 0x49, 0x68, 0x68, 0xe8, 0x90, 0x21, 0x43, 0xa6, 0x4f, 
  0x9f, 0x1e, 0x1e, 0x1e, 0x4e, 0x54, 0x1b, 0x5f, 0x80, 0x22, 0xc6, 0x50, 0x28, 0x14, 0xd1, 0xd1, 
  0xd1, 0x4d, 0x9a, 0x34, 0x61, 0xdd, 0xb0, 0x0b, 0x17, 0x2e, 0xbc, 0x7b, 0xf7, 0x2e, 0xed, 0x7a, 
  0x10, 0x47, 0x34, 0x90, 0x11, 0x8e, 0x1d, 0x3b, 0xe6, 0xe2, 0xe2, 0x82, 0xe5, 0xff, 0x73, 0xed, 
  0xda, 0xb5, 0xb4, 0x6b, 0x43, 0x16, 0xd1, 0x40, 0xff, 0x41, 0xad, 0x56, 0x4f, 0x9d, 0x3a, 0x15, 
  0x8b, 0x75, 0xb4, 0xf4, 0xeb, 0xd7, 0xaf, 0xac, 0xac, 0x8c, 0x76, 0xcd, 0x48, 0x21, 0x1a, 0xe8, 
  0xff, 0x28, 0x14, 0x8a, 0x88, 0x88, 0x08, 0xbc, 0xee, 0xd1, 0x20, 0x97, 0xcb, 0x6f, 0xdf, 0xbe, 
  0x4d, 0xbb, 0x7e, 0x44, 0x10, 0x6f, 0xa2, 0xff, 0x45, 0xa1, 0x50, 0x84, 0x87, 0x87, 0x3f, 0x79, 
  0xf2, 0x84, 0x50, 0xfe, 0x8e, 0x8e, 0x8e, 0xe9, 0xe9, 0xe9, 0x61, 0x61, 0x61, 0x84, 0xf2, 0xa7, 
  0x85, 0x68, 0x20, 0x00, 0x00, 0x28, 0x2b, 0x2b, 0xeb, 0xd0, 0xa1, 0x43, 0x49, 0x49, 0x09, 0xd1, 
  0x52, 0x1c, 0x1d, 0x1d, 0xef, 0xdd, 0xbb, 0xd7, 0xba, 0x75, 0x6b, 0xa2, 0xa5, 0x58, 0x18, 0x71, 
  0x24, 0x1a, 0x00, 0x00, 0x86, 0x0c, 0x19, 0x42, 0xda, 0x3d, 0x00, 0x80, 0xfa, 0xfa, 0xfa, 0x01, 
  0x03, 0x06, 0x54, 0x54, 0x54, 0x90, 0x2e, 0xc8, 0x92, 0x88, 0x06, 0x02, 0x93, 0x27, 0x4f, 0xce, 
  0xc8, 0xc8, 0xb0, 0x4c, 0x59, 0x05, 0x05, 0x05, 0x13, 0x26, 0x4c, 0xb0, 0x4c, 0x59, 0x96, 0xc1, 
  0xd6, 0x0d, 0x94, 0x94, 0x94, 0x94, 0x98, 0x98, 0x68, 0xc9, 0x12, 0x2f, 0x5d, 0xba, 0xb4, 0x7d, 
  0xfb, 0x76, 0x4b, 0x96, 0x48, 0x14, 0x9b, 0xbe, 0x07, 0x7a, 0xf4, 0xe8, 0x51, 0x48, 0x48, 0xc8, 
  0xb3, 0x67, 0xcf, 0x2c, 0x5f, 0x74, 0x79, 0x79, 0xf9, 0xab, 0xaf, 0xbe, 0x6a, 0xf9, 0x72, 0xb1, 
  0x63, 0xd3, 0x3d, 0xd0, 0xfa, 0xf5, 0xeb, 0xa9, 0xb8, 0x47, 0x53, 0x34, 0x95, 0x72, 0xb1, 0x63, 
  0xbb, 0x3d, 0x90, 0x52, 0xa9, 0x94, 0xc9, 0x64, 0xcf, 0x9f, 0x3f, 0xa7, 0x52, 0xba, 0x9d, 0x9d, 
  0x9d, 0x42, 0xa1, 0xf0, 0xf7, 0xf7, 0xa7, 0x52, 0x3a, 0x46, 0x6c, 0xb7, 0x07, 0xda, 0xbb, 0x77, 
  0x2f, 0x2d, 0xf7, 0x00, 0x00, 0x20, 0x84, 0x7b, 0xf6, 0xec, 0xa1, 0x55, 0x3a, 0x46, 0x6c, 0xb7, 
  0x07, 0x0a, 0x0c, 0x0c, 0x2c, 0x28, 0x28, 0xa0, 0x28, 0x20, 0x2c, 0x2c, 0xec, 0xee, 0xdd, 0xbb, 
  0x14, 0x05, 0x60, 0xc1, 0x46, 0x7b, 0xa0, 0x7b, 0xf7, 0xee, 0xd1, 0x75, 0x0f, 0x00, 0x20, 0x33, 
  0x33, 0x33, 0x2b, 0x2b, 0x8b, 0xae, 0x06, 0xee, 0xd8, 0xa8, 0x81, 0x0e, 0x1d, 0x3a, 0x44, 0x5b, 
  0x02, 0x00, 0x00, 0xfc, 0xfc, 0xf3, 0xcf, 0xb4, 0x25, 0x70, 0xc5, 0x16, 0x0d, 0x54, 0x5f, 0x5f, 
  0xbf, 0x6b, 0xd7, 0x2e, 0xda, 0x2a, 0x00, 0x00, 0xe0, 0xca, 0x95, 0x2b, 0xb4, 0x25, 0x70, 0x85, 
  0xb8, 0x81, 0xca, 0xca, 0xca, 0x72, 0x72, 0x72, 0xca, 0xcb, 0xcb, 0x49, 0x17, 0x84, 0xce, 0xe9, 
  0xd3, 0xa7, 0x79, 0xf2, 0x3e, 0xa1, 0xa8, 0xa8, 0x88, 0xb6, 0x04, 0xae, 0x90, 0x35, 0xd0, 0xe4, 
  0xc9, 0x93, 0xe5, 0x72, 0x79, 0xfb, 0xf6, 0xed, 0xe5, 0x72, 0x79, 0x64, 0x64, 0x64, 0x72, 0x72, 
  0x32, 0xd1, 0xe2, 0x10, 0xe1, 0xcf, 0x85, 0xa3, 0xb2, 0xb2, 0xf2, 0xc5, 0x8b, 0x17, 0x24, 0x72, 
  0xce, 0xc9, 0xc9, 0x39, 0x77, 0xee, 0x5c, 0x5e, 0x5e, 0x1e, 0x89, 0xcc, 0xff, 0x03, 0xb9, 0x99, 
  0x22, 0x51, 0x51, 0x51, 0x86, 0xc5, 0xcd, 0x9e, 0x3d, 0xbb, 0xa2, 0xa2, 0x82, 0x5c, 0xa1, 0x8d, 
  0x52, 0x57, 0x57, 0x27, 0x93, 0xc9, 0x88, 0x37, 0x2b, 0x1a, 0x12, 0x89, 0x44, 0xa1, 0x50, 0x60, 
  0xac, 0x5d, 0x71, 0x71, 0xf1, 0x96, 0x2d, 0x5b, 0xba, 0x76, 0xed, 0xaa, 0x2d, 0x62, 0xce, 0x9c, 
  0x39, 0x2f, 0x5e, 0xbc, 0xc0, 0x58, 0x84, 0x1e, 0xa4, 0x0c, 0x14, 0x1f, 0x1f, 0x6f, 0xaa, 0xd5, 
  0xfc, 0xfc, 0xfc, 0x6e, 0xdc, 0xb8, 0x41, 0xa8, 0xdc, 0x46, 0xf9, 0xe1, 0x87, 0x1f, 0x2c, 0xe2, 
  0x0d, 0x54, 0xee, 0xdd, 0xbb, 0x87, 0xab, 0x6a, 0x9b, 0x37, 0x6f, 0x96, 0x4a, 0xa5, 0x86, 0x45, 
  0x0c, 0x19, 0x32, 0x04, 0x57, 0x11, 0x86, 0x90, 0x32, 0x90, 0x5c, 0x2e, 0x67, 0x6e, 0xb8, 0x98, 
  0x98, 0x18, 0x42, 0x45, 0x33, 0xd3, 0xa9, 0x53, 0x27, 0xa3, 0x7a, 0xb8, 0xcc, 0x9f, 0x47, 0xc4, 
  0xd1, 0xd1, 0xd1, 0xf0, 0xcb, 0xac, 0xac, 0x2c, 0xee, 0x95, 0x4a, 0x48, 0x48, 0x08, 0x0a, 0x0a, 
  0x62, 0x28, 0x7a, 0xdb, 0xb6, 0x6d, 0xdc, 0x4b, 0x31, 0x0a, 0x11, 0x03, 0x21, 0x5e, 0x7a, 0xc7, 
  0x8f, 0x1f, 0x9f, 0x9f, 0x9f, 0x4f, 0x42, 0x80, 0x29, 0x0a, 0x0b, 0x0b, 0x4d, 0x89, 0xe9, 0xd0, 
  0xa1, 0x43, 0x72, 0x72, 0x72, 0x9b, 0x36, 0x6d, 0x50, 0x94, 0x9b, 0xcb, 0xd0, 0xa1, 0x43, 0x0f, 
  0x1e, 0x3c, 0xe8, 0xec, 0xec, 0x6c, 0xf8, 0x13, 0xf7, 0x1e, 0x68, 0xc9, 0x92, 0x25, 0x8d, 0x0a, 
  0xf0, 0xf0, 0xf0, 0xc0, 0xd2, 0x80, 0x86, 0x10, 0x31, 0xd0, 0x57, 0x5f, 0x7d, 0x85, 0xd8, 0xb2, 
  0x8e, 0x8e, 0x8e, 0x71, 0x71, 0x71, 0x24, 0x34, 0x18, 0x65, 0xd3, 0xa6, 0x4d, 0xa6, 0x94, 0xb4, 
  0x6d, 0xdb, 0x16, 0x42, 0xf8, 0xf2, 0xe5, 0xcb, 0xc1, 0x83, 0x07, 0x23, 0x8a, 0x47, 0x64, 0xca, 
  0x94, 0x29, 0x9a, 0xd2, 0x0d, 0x7f, 0xe2, 0x78, 0x0f, 0xa4, 0x50, 0x28, 0x7a, 0xf4, 0xe8, 0x81, 
  0x28, 0xe3, 0xdc, 0xb9, 0x73, 0x78, 0x1a, 0xf1, 0xbf, 0x10, 0x31, 0xd0, 0x80, 0x01, 0x03, 0xcc, 
  0x6a, 0xe2, 0x9e, 0x3d, 0x7b, 0xfe, 0xf3, 0xcf, 0x3f, 0x24, 0x94, 0xe8, 0xf1, 0xfa, 0xeb, 0xaf, 
  0x9b, 0xd2, 0xd0, 0xba, 0x75, 0x6b, 0x95, 0x4a, 0xa5, 0x39, 0x6c, 0xed, 0xda, 0xb5, 0x66, 0xe9, 
  0x67, 0x60, 0xef, 0xde, 0xbd, 0x9a, 0x3c, 0xd3, 0xd2, 0xd2, 0x0c, 0x7f, 0x75, 0x77, 0x77, 0xaf, 
  0xaa, 0xaa, 0x62, 0x57, 0x97, 0xb4, 0xb4, 0x34, 0x37, 0x37, 0x37, 0x74, 0x25, 0x91, 0x91, 0x91, 
  0x78, 0x1a, 0xf1, 0xbf, 0xe0, 0x37, 0x50, 0x4d, 0x4d, 0x8d, 0xa7, 0xa7, 0xa7, 0xb9, 0x0d, 0xed, 
  0xe1, 0xe1, 0xb1, 0x79, 0xf3, 0x66, 0xec, 0x62, 0x74, 0x49, 0x4f, 0x4f, 0x67, 0x10, 0x20, 0x95, 
  0x4a, 0x75, 0x17, 0xdf, 0x9c, 0x39, 0x73, 0x26, 0x30, 0x30, 0xd0, 0xdc, 0x5a, 0xe8, 0xd2, 0xb5, 
  0x6b, 0x57, 0xdd, 0x67, 0x05, 0xa3, 0x43, 0x18, 0xad, 0x5a, 0xb5, 0x52, 0xab, 0xd5, 0x2c, 0xea, 
  0xc2, 0x6e, 0x24, 0xfd, 0xd1, 0xa3, 0x47, 0x18, 0xda, 0xf1, 0xbf, 0xe0, 0x37, 0x90, 0xd1, 0x7f, 
  0x35, 0x44, 0xc6, 0x8f, 0x1f, 0x5f, 0x54, 0x54, 0x84, 0x5d, 0x92, 0x86, 0xcf, 0x3f, 0xff, 0x9c, 
  0xb9, 0xf4, 0x0b, 0x17, 0x2e, 0xe8, 0x1e, 0x5f, 0x55, 0x55, 0x35, 0x62, 0xc4, 0x08, 0x76, 0x15, 
  0x99, 0x39, 0x73, 0xa6, 0x5e, 0xe9, 0xdf, 0x7e, 0xfb, 0xad, 0xe1, 0x61, 0x11, 0x11, 0x11, 0x2c, 
  0x2a, 0xb2, 0x65, 0xcb, 0x16, 0x76, 0xaa, 0x8e, 0x1c, 0x39, 0xc2, 0xba, 0xf5, 0x4c, 0x81, 0xdf, 
  0x40, 0x09, 0x09, 0x09, 0xec, 0xaa, 0xa7, 0xc1, 0xc1, 0xc1, 0x61, 0xcf, 0x9e, 0x3d, 0xd8, 0x55, 
  0x41, 0x84, 0x07, 0xc3, 0x45, 0x8b, 0x16, 0x19, 0xa6, 0x3a, 0x7c, 0xf8, 0xb0, 0xd1, 0x67, 0x63, 
  0x53, 0xf8, 0xfa, 0xfa, 0xa6, 0xa4, 0xa4, 0x18, 0xe6, 0xb3, 0x60, 0xc1, 0x02, 0xc3, 0x83, 0x59, 
  0x3c, 0x60, 0xcf, 0x98, 0x31, 0xc3, 0xbc, 0x06, 0xd5, 0x61, 0xc9, 0x92, 0x25, 0x2c, 0xda, 0x8d, 
  0x19, 0xfc, 0x06, 0xfa, 0xf2, 0xcb, 0x2f, 0x59, 0xd7, 0x50, 0xcb, 0xf0, 0xe1, 0xc3, 0x73, 0x73, 
  0x73, 0x31, 0xaa, 0x3a, 0x7b, 0xf6, 0x6c, 0xa3, 0x85, 0x7a, 0x7b, 0x7b, 0x1b, 0x4d, 0x5b, 0x50, 
  0x50, 0xb0, 0x68, 0xd1, 0x22, 0x14, 0xd9, 0xcb, 0x97, 0x2f, 0x7f, 0xfa, 0xf4, 0xa9, 0x61, 0x0e, 
  0x15, 0x15, 0x15, 0x46, 0xef, 0x57, 0x66, 0xcd, 0x9a, 0x85, 0x5e, 0x05, 0x95, 0x4a, 0x35, 0x76, 
  0xec, 0x58, 0xb3, 0x9b, 0x52, 0x87, 0xa0, 0xa0, 0x20, 0x96, 0xcd, 0x67, 0x1a, 0xfc, 0x06, 0x9a, 
  0x38, 0x71, 0x22, 0x97, 0x4a, 0x6a, 0x91, 0x48, 0x24, 0xbb, 0x76, 0xed, 0xc2, 0xa5, 0x6a, 0xe4, 
  0xc8, 0x91, 0x28, 0x85, 0x32, 0x74, 0xf2, 0x97, 0x2f, 0x5f, 0xee, 0xd7, 0xaf, 0x9f, 0xa9, 0x84, 
  0x23, 0x46, 0x8c, 0x48, 0x4f, 0x4f, 0x37, 0x95, 0x76, 0xd9, 0xb2, 0x65, 0x46, 0x53, 0xad, 0x5a, 
  0xb5, 0x0a, 0x51, 0x7f, 0x71, 0x71, 0x31, 0xfa, 0x03, 0x17, 0x03, 0x19, 0x19, 0x19, 0x66, 0xb7, 
  0x1d, 0x23, 0xf8, 0x0d, 0x14, 0x1c, 0x1c, 0xcc, 0xbd, 0x9e, 0x5a, 0xba, 0x76, 0xed, 0x5a, 0x58, 
  0x58, 0xc8, 0x5d, 0x55, 0xd3, 0xa6, 0x4d, 0x51, 0x8a, 0x73, 0x71, 0x71, 0xa9, 0xac, 0xac, 0x64, 
  0xc8, 0x27, 0x35, 0x35, 0x35, 0x2a, 0x2a, 0xaa, 0x65, 0xcb, 0x96, 0xf6, 0xf6, 0xf6, 0xf6, 0xf6, 
  0xf6, 0xad, 0x5b, 0xb7, 0x9e, 0x39, 0x73, 0xe6, 0x9d, 0x3b, 0x77, 0x18, 0x92, 0x14, 0x15, 0x15, 
  0xbd, 0xf2, 0xca, 0x2b, 0x46, 0x8b, 0xdb, 0xb9, 0x73, 0x27, 0x8a, 0xf8, 0x9c, 0x9c, 0x1c, 0x77, 
  0x77, 0x77, 0x2c, 0xed, 0x89, 0x3d, 0xd8, 0x03, 0x66, 0x03, 0x15, 0x16, 0x16, 0xda, 0xd9, 0xd9, 
  0x61, 0xa9, 0xaa, 0x16, 0xa9, 0x54, 0x1a, 0x1b, 0x1b, 0xcb, 0x45, 0xd5, 0xc5, 0x8b, 0x17, 0xd1, 
  0x8b, 0x8b, 0x88, 0x88, 0x78, 0xf9, 0xf2, 0x25, 0x73, 0x86, 0xa7, 0x4e, 0x9d, 0x72, 0x74, 0x74, 
  0x94, 0x48, 0x24, 0xbf, 0xfd, 0xf6, 0x1b, 0xf3, 0x91, 0x25, 0x25, 0x25, 0x0c, 0x13, 0x9f, 0x8f, 
  0x1f, 0x3f, 0xde, 0xa8, 0xf8, 0xc4, 0xc4, 0x44, 0x8c, 0xa3, 0xe4, 0x63, 0xc6, 0x8c, 0x31, 0xa3, 
  0xe1, 0x10, 0xc0, 0x6c, 0xa0, 0x63, 0xc7, 0x8e, 0xe1, 0xaa, 0xaa, 0x1e, 0x7d, 0xfb, 0xf6, 0xbd, 
  0x7e, 0xfd, 0x3a, 0x3b, 0x55, 0xfd, 0xfb, 0xf7, 0x37, 0xab, 0xac, 0xd0, 0xd0, 0x50, 0xa3, 0x37, 
  0xc2, 0x10, 0xc2, 0x3b, 0x77, 0xee, 0xc4, 0xc4, 0xc4, 0xf8, 0xf9, 0xf9, 0x69, 0x8e, 0x94, 0xcb, 
  0xe5, 0x1b, 0x36, 0x6c, 0x30, 0xf5, 0xe4, 0x78, 0xf2, 0xe4, 0xc9, 0x16, 0x2d, 0x5a, 0x30, 0x14, 
  0xf4, 0xfb, 0xef, 0xbf, 0x33, 0x2b, 0xc7, 0x38, 0x22, 0xa5, 0x21, 0x24, 0x24, 0x84, 0x5d, 0x1b, 
  0x9a, 0x02, 0xb3, 0x81, 0xbe, 0xf8, 0xe2, 0x0b, 0xbc, 0x15, 0xd6, 0xe3, 0x9b, 0x6f, 0xbe, 0x31, 
  0x57, 0x12, 0xeb, 0xa9, 0xab, 0x51, 0x51, 0x51, 0xa7, 0x4e, 0x9d, 0xca, 0xcd, 0xcd, 0xbd, 0x7f, 
  0xff, 0x7e, 0x7a, 0x7a, 0xfa, 0xde, 0xbd, 0x7b, 0x4d, 0x0d, 0x52, 0x3b, 0x38, 0x38, 0x4c, 0x9c, 
  0x38, 0xf1, 0xfb, 0xef, 0xbf, 0xbf, 0x7d, 0xfb, 0x76, 0x41, 0x41, 0x41, 0x76, 0x76, 0xf6, 0xe1, 
  0xc3, 0x87, 0x51, 0x5c, 0x9b, 0x97, 0x97, 0xc7, 0xa0, 0xfc, 0x83, 0x0f, 0x3e, 0x60, 0xa7, 0x9c, 
  0x01, 0xa9, 0x54, 0x6a, 0xf4, 0x36, 0x9f, 0x35, 0x98, 0x0d, 0x34, 0x7e, 0xfc, 0x78, 0xec, 0x75, 
  0xd6, 0x23, 0x3c, 0x3c, 0xfc, 0xea, 0xd5, 0xab, 0xe8, 0x92, 0xb8, 0x4f, 0x3e, 0xb4, 0xb7, 0x37, 
  0x63, 0xd6, 0x14, 0xfa, 0xc1, 0x32, 0x99, 0x8c, 0x61, 0x66, 0xcb, 0xd0, 0xa1, 0x43, 0x59, 0x89, 
  0x6d, 0x9c, 0xd4, 0xd4, 0x54, 0x1c, 0xa7, 0xfa, 0x5f, 0x30, 0x1b, 0x48, 0x77, 0x26, 0x0a, 0x51, 
  0x62, 0x62, 0x62, 0xb4, 0x6f, 0x1e, 0x98, 0x61, 0x78, 0x74, 0xa2, 0x8b, 0x5c, 0x2e, 0x37, 0x5a, 
  0x85, 0x7f, 0xfe, 0xf9, 0xa7, 0x77, 0xef, 0xde, 0xe4, 0xca, 0xdd, 0xb7, 0x6f, 0x1f, 0xc6, 0x33, 
  0x8e, 0xd3, 0x40, 0x6a, 0xb5, 0x3a, 0x20, 0x20, 0x80, 0x5c, 0xcd, 0xf5, 0xe8, 0xd0, 0xa1, 0x43, 
  0x66, 0x66, 0x26, 0xb3, 0xa4, 0x9c, 0x9c, 0x1c, 0x8b, 0xe9, 0x31, 0x97, 0x8e, 0x1d, 0x3b, 0x1a, 
  0x0a, 0xce, 0xcb, 0xcb, 0x23, 0xbd, 0xe4, 0x79, 0xc5, 0x8a, 0x15, 0x18, 0x4f, 0x3a, 0x4e, 0x03, 
  0x3d, 0x7c, 0xf8, 0x50, 0x22, 0x91, 0x10, 0xad, 0xbc, 0xb9, 0xcd, 0xb1, 0x78, 0xf1, 0x62, 0x0b, 
  0xeb, 0x41, 0xa7, 0x6f, 0xdf, 0xbe, 0x7a, 0x6a, 0x93, 0x92, 0x92, 0x2c, 0x30, 0x2d, 0x69, 0xd2, 
  0xa4, 0x49, 0x18, 0x4f, 0x3a, 0x4e, 0x03, 0x31, 0xbf, 0xad, 0x24, 0x47, 0x64, 0x64, 0xa4, 0xa9, 
  0xeb, 0x3a, 0xc7, 0x17, 0xa2, 0x44, 0x19, 0x35, 0x6a, 0x94, 0xae, 0x54, 0xa3, 0x2f, 0xcb, 0x48, 
  0xd0, 0xb9, 0x73, 0x67, 0x8c, 0x27, 0x1d, 0xa7, 0x81, 0x4e, 0x9d, 0x3a, 0x65, 0x99, 0x26, 0x30, 
  0xca, 0xba, 0x75, 0xeb, 0xf4, 0xde, 0x6c, 0xf3, 0x7c, 0xd9, 0x9e, 0x76, 0x9e, 0x10, 0x44, 0x78, 
  0xd1, 0x8b, 0x11, 0x57, 0x57, 0x57, 0xe6, 0xc1, 0x52, 0xb3, 0xc0, 0x69, 0xa0, 0xad, 0x5b, 0xb7, 
  0x5a, 0xac, 0x15, 0x8c, 0xd2, 0xae, 0x5d, 0x3b, 0xdd, 0x19, 0x14, 0xf3, 0xe7, 0xcf, 0xa7, 0xab, 
  0x87, 0x99, 0x39, 0x73, 0xe6, 0x68, 0x74, 0x22, 0xbe, 0x66, 0xc1, 0x48, 0x76, 0x76, 0x36, 0xae, 
  0x93, 0x8e, 0xd3, 0x40, 0x1f, 0x7e, 0xf8, 0xa1, 0x85, 0x1b, 0xc2, 0x28, 0x2b, 0x57, 0xae, 0xd4, 
  0xe8, 0x31, 0x3a, 0x85, 0x94, 0x3f, 0x68, 0x9e, 0x86, 0xba, 0x74, 0xe9, 0x62, 0xf9, 0xa2, 0x2f, 
  0x5d, 0xba, 0x84, 0xeb, 0xa4, 0xe3, 0xdc, 0x2b, 0xe3, 0xfe, 0xfd, 0xfb, 0x18, 0x73, 0x63, 0x4d, 
  0x6c, 0x6c, 0x6c, 0x51, 0x51, 0x51, 0xd7, 0xae, 0x5d, 0x6b, 0x6a, 0x6a, 0x68, 0x6b, 0x61, 0x22, 
  0x2d, 0x2d, 0xed, 0xe0, 0xc1, 0x83, 0xe8, 0x9b, 0x28, 0x60, 0xa4, 0xac, 0xac, 0x0c, 0x5b, 0x5e, 
  0xb8, 0x9c, 0x08, 0x4d, 0x2f, 0x78, 0x10, 0xe1, 0x1b, 0xda, 0x89, 0xb6, 0xdc, 0xc1, 0xb6, 0x32, 
  0xb5, 0xa6, 0xa6, 0xa6, 0xb8, 0xb8, 0x18, 0x57, 0x6e, 0x22, 0x44, 0xa9, 0xac, 0xac, 0xc4, 0x95, 
  0x15, 0x36, 0x03, 0x3d, 0x7a, 0xf4, 0xa8, 0xb4, 0xb4, 0x14, 0x57, 0x6e, 0x22, 0x44, 0xc1, 0x18, 
  0x1a, 0x00, 0x9b, 0x81, 0x78, 0x12, 0xae, 0x40, 0x04, 0x05, 0x8c, 0xa1, 0xd9, 0xb0, 0x19, 0x08, 
  0x63, 0xaf, 0x28, 0xc2, 0x0e, 0x37, 0x37, 0x37, 0xa3, 0x8b, 0x5f, 0x0d, 0xe1, 0x63, 0x0f, 0x54, 
  0x55, 0x55, 0x85, 0x2b, 0x2b, 0x11, 0x76, 0x44, 0x45, 0x45, 0x75, 0xeb, 0xd6, 0x0d, 0xe5, 0x48, 
  0x3e, 0xde, 0x03, 0x55, 0x57, 0x57, 0xe3, 0xca, 0x4a, 0x84, 0x1d, 0xdd, 0xbb, 0x77, 0xef, 0xd5, 
  0xab, 0x17, 0xca, 0x91, 0x7c, 0xbc, 0x84, 0x51, 0x8c, 0x78, 0x2a, 0xa2, 0xc1, 0xcb, 0xcb, 0x6b, 
  0xca, 0x94, 0x29, 0x28, 0x47, 0x62, 0xfc, 0x6f, 0xc7, 0x66, 0x20, 0x42, 0x81, 0x92, 0x44, 0xd0, 
  0x51, 0x2a, 0x95, 0x61, 0x61, 0x61, 0x28, 0xd1, 0x8f, 0x94, 0x4a, 0x25, 0xae, 0x42, 0x69, 0x5e, 
  0xc2, 0x22, 0x22, 0x22, 0x58, 0x2f, 0xfd, 0x14, 0x31, 0x44, 0xb3, 0x9c, 0x61, 0xf4, 0xe8, 0xd1, 
  0x8d, 0x1e, 0x59, 0x5b, 0x5b, 0xdb, 0xd0, 0xd0, 0x80, 0xa5, 0x50, 0x6c, 0x06, 0x62, 0xb1, 0x67, 
  0xc0, 0xc5, 0x8b, 0x17, 0xe3, 0xe2, 0xe2, 0x70, 0x09, 0x10, 0xd1, 0xf4, 0x3d, 0x28, 0x33, 0x30, 
  0xeb, 0xeb, 0xeb, 0x71, 0x75, 0x42, 0xd4, 0x2e, 0x61, 0x6f, 0xbe, 0xf9, 0xa6, 0xbb, 0xbb, 0x3b, 
  0xce, 0x97, 0x32, 0x36, 0x8f, 0x26, 0xa6, 0x45, 0xff, 0xfe, 0xfd, 0x1b, 0x9d, 0x97, 0xad, 0x52, 
  0xa9, 0x54, 0x2a, 0x15, 0x96, 0x42, 0xa9, 0x5d, 0xc2, 0x62, 0x62, 0x62, 0x00, 0x00, 0xe2, 0xe0, 
  0x35, 0x2e, 0x24, 0x12, 0x89, 0xc6, 0x40, 0xfe, 0xfe, 0xfe, 0x8d, 0x4e, 0xa9, 0xd6, 0xbc, 0xc6, 
  0xc2, 0x52, 0x2e, 0x36, 0x03, 0xd5, 0xd7, 0xd7, 0xa3, 0x1f, 0xdc, 0xbb, 0x77, 0x6f, 0x4d, 0x4f, 
  0xfb, 0xf2, 0xe5, 0x4b, 0x5c, 0x02, 0x6c, 0x1c, 0x6f, 0x6f, 0x6f, 0x6f, 0x6f, 0x6f, 0xcd, 0xe7, 
  0x31, 0x63, 0xc6, 0x30, 0x1f, 0x6c, 0x67, 0x67, 0x67, 0xd6, 0x52, 0x13, 0x06, 0x70, 0xbe, 0x4c, 
  0x45, 0x3f, 0xf8, 0xe3, 0x8f, 0x3f, 0xd6, 0x7c, 0x10, 0x9f, 0xdd, 0x70, 0x21, 0x93, 0xc9, 0x9c, 
  0x9c, 0x9c, 0x34, 0x9f, 0xc7, 0x8c, 0x19, 0xc3, 0xbc, 0x3e, 0x58, 0x22, 0x91, 0x38, 0x38, 0xe0, 
  0x99, 0xc9, 0x43, 0x61, 0x1c, 0x28, 0x20, 0x20, 0x40, 0xfb, 0xa4, 0x20, 0x8e, 0x1e, 0xe1, 0x42, 
  0xf7, 0xe9, 0x3d, 0x20, 0x20, 0x80, 0x79, 0x32, 0xb8, 0x93, 0x93, 0x13, 0xae, 0xe5, 0x0f, 0x14, 
  0x7a, 0x20, 0xdd, 0x08, 0x8a, 0x4f, 0x9f, 0x3e, 0xc5, 0x25, 0xc0, 0xc6, 0xd1, 0x5e, 0xbf, 0x34, 
  0xf4, 0xec, 0xd9, 0x93, 0xe1, 0x60, 0x47, 0x47, 0x47, 0xde, 0x5d, 0xc2, 0x10, 0xef, 0x81, 0x5c, 
  0x5d, 0x5d, 0xa7, 0x4d, 0x9b, 0xa6, 0xfd, 0x53, 0xbc, 0x84, 0xe1, 0xc2, 0xc7, 0xc7, 0x47, 0xf7, 
  0x4f, 0xdd, 0x46, 0x36, 0x04, 0xe3, 0x64, 0x5f, 0x3c, 0x06, 0x52, 0xa9, 0x54, 0x88, 0x03, 0x53, 
  0x33, 0x67, 0xce, 0xd4, 0x8d, 0x75, 0xc2, 0xf3, 0x59, 0xa7, 0x02, 0xe2, 0xb5, 0xd7, 0x5e, 0xd3, 
  0xfd, 0x73, 0xe0, 0xc0, 0x81, 0x2d, 0x5b, 0xb6, 0x34, 0x75, 0x30, 0x62, 0xb0, 0x1b, 0x14, 0xb0, 
  0x19, 0x48, 0xad, 0x56, 0x37, 0x5e, 0x98, 0xbd, 0x7d, 0x74, 0x74, 0xb4, 0xee, 0x37, 0xa2, 0x81, 
  0x70, 0xd1, 0xae, 0x5d, 0x3b, 0xbd, 0x6f, 0x26, 0x4f, 0x9e, 0x6c, 0xea, 0xe0, 0x66, 0xcd, 0x9a, 
  0xe1, 0x2a, 0x17, 0x8f, 0x81, 0x10, 0xc7, 0x15, 0x16, 0x2c, 0x58, 0xe0, 0xe5, 0xe5, 0xa5, 0xfb, 
  0x8d, 0xf8, 0x18, 0x8f, 0x05, 0xa9, 0x54, 0x6a, 0x38, 0xf6, 0xc3, 0x10, 0x1e, 0xc4, 0xc3, 0xc3, 
  0x03, 0x57, 0xd1, 0x78, 0x0c, 0x64, 0x67, 0x67, 0x87, 0x12, 0x57, 0xca, 0x70, 0xdd, 0x0f, 0xc6, 
  0xb7, 0x7a, 0xb6, 0x4c, 0x54, 0x54, 0x94, 0x61, 0x08, 0xb3, 0x3e, 0x7d, 0xfa, 0xe8, 0xdd, 0x18, 
  0x69, 0xe1, 0x9d, 0x81, 0x50, 0xe8, 0xdf, 0xbf, 0xbf, 0x61, 0xf4, 0xbb, 0x90, 0x90, 0x10, 0x76, 
  0xb9, 0x49, 0x24, 0x12, 0x67, 0x67, 0x67, 0xcb, 0x2f, 0xc5, 0xe7, 0x27, 0xf3, 0xe6, 0xcd, 0x33, 
  0xfc, 0xd2, 0xc9, 0xc9, 0xc9, 0xd4, 0x2e, 0x08, 0x18, 0x0d, 0x84, 0x67, 0x59, 0x4f, 0x43, 0x43, 
  0x43, 0xa3, 0xab, 0xd0, 0xef, 0xde, 0xbd, 0x6b, 0x98, 0xb0, 0xa2, 0xa2, 0x42, 0x3b, 0xfc, 0xc5, 
  0x80, 0xb7, 0xb7, 0xf7, 0xc8, 0x91, 0x23, 0x57, 0xaf, 0x5e, 0x7d, 0xf8, 0xf0, 0xe1, 0xdf, 0x7e, 
  0xfb, 0x2d, 0x3b, 0x3b, 0x5b, 0xa1, 0x50, 0x3c, 0x7e, 0xfc, 0x58, 0xa1, 0x50, 0x64, 0x64, 0x64, 
  0xec, 0xdc, 0xb9, 0x93, 0xb5, 0x11, 0xad, 0x00, 0xed, 0x0a, 0x57, 0x43, 0x4c, 0x6d, 0x68, 0xb7, 
  0x63, 0xc7, 0x0e, 0x2c, 0xe7, 0x1d, 0xe2, 0x5a, 0x99, 0xaa, 0x56, 0xab, 0x99, 0x4f, 0x61, 0x58, 
  0x58, 0x98, 0xa9, 0xb4, 0x46, 0xb7, 0x7f, 0x93, 0x48, 0x24, 0x3d, 0x7a, 0xf4, 0x98, 0x35, 0x6b, 
  0x56, 0x6c, 0x6c, 0x6c, 0x4a, 0x4a, 0x4a, 0x75, 0x75, 0x35, 0xb3, 0x80, 0x86, 0x86, 0x06, 0xa3, 
  0x81, 0x98, 0xad, 0x9e, 0xb9, 0x73, 0xe7, 0x32, 0xb7, 0x8c, 0xd1, 0x01, 0xa1, 0x43, 0x87, 0x0e, 
  0xb1, 0x39, 0xcd, 0xc6, 0xc0, 0xb6, 0xb0, 0x90, 0x61, 0x1b, 0x0a, 0x00, 0x40, 0x62, 0x62, 0x22, 
  0x43, 0xda, 0xf4, 0xf4, 0xf4, 0xe1, 0xc3, 0x87, 0xfb, 0xf9, 0xf9, 0xc9, 0xe5, 0xf2, 0x49, 0x93, 
  0x26, 0xc5, 0xc7, 0xc7, 0xb3, 0x8b, 0x57, 0x8f, 0x7d, 0x9f, 0x14, 0xde, 0xe2, 0xe3, 0xe3, 0x33, 
  0x61, 0xc2, 0x04, 0x94, 0xa0, 0x91, 0x7f, 0xfc, 0xf1, 0x87, 0x61, 0x72, 0x8c, 0x1b, 0xaf, 0x60, 
  0xdb, 0x37, 0xbe, 0x73, 0xe7, 0xce, 0x37, 0x6f, 0xde, 0x34, 0xfa, 0x53, 0x78, 0x78, 0x78, 0x46, 
  0x46, 0x46, 0xa3, 0x39, 0xd4, 0xd5, 0xd5, 0x39, 0x3a, 0x3a, 0x72, 0x09, 0xf2, 0x9a, 0x9d, 0x9d, 
  0xcd, 0xec, 0x63, 0xa1, 0x20, 0x91, 0x48, 0xfc, 0xfd, 0xfd, 0xfd, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 
  0xbc, 0xbd, 0xbd, 0x3d, 0x3d, 0x3d, 0x3d, 0x3c, 0x3c, 0xdc, 0xdd, 0xdd, 0x3d, 0x3c, 0x3c, 0x34, 
  0x7f, 0xfa, 0xfb, 0xfb, 0xa3, 0x0f, 0x06, 0x76, 0xef, 0xde, 0x5d, 0x6f, 0xff, 0x89, 0xac, 0xac, 
  0xac, 0xf6, 0xed, 0xdb, 0xe3, 0xd1, 0x8a, 0xcb, 0x89, 0x0c, 0x53, 0x08, 0x0e, 0x1c, 0x38, 0x80, 
  0xab, 0x94, 0x46, 0x41, 0x5c, 0x96, 0xc0, 0x43, 0x64, 0x32, 0xd9, 0xd4, 0xa9, 0x53, 0xb7, 0x6d, 
  0xdb, 0x96, 0x92, 0x92, 0x52, 0x50, 0x50, 0x50, 0x57, 0x57, 0x87, 0xab, 0x4d, 0x4e, 0x9e, 0x3c, 
  0xa9, 0x5b, 0x50, 0xf3, 0xe6, 0xcd, 0x31, 0x66, 0x8e, 0xcd, 0x40, 0xa6, 0x82, 0x42, 0xba, 0xbb, 
  0xbb, 0x13, 0xdd, 0xb3, 0x53, 0x8f, 0xd9, 0xb3, 0x67, 0x5b, 0xe4, 0x74, 0xe3, 0x44, 0x22, 0x91, 
  0x2c, 0x59, 0xb2, 0x84, 0xe8, 0x56, 0xb2, 0xba, 0x67, 0x67, 0xf2, 0xe4, 0xc9, 0x18, 0x73, 0xc6, 
  0x16, 0x9d, 0xc3, 0xd4, 0x92, 0xb6, 0x8d, 0x1b, 0x37, 0xf2, 0x3c, 0xcc, 0x0a, 0x5d, 0xc2, 0xc2, 
  0xc2, 0x8e, 0x1f, 0x3f, 0x4e, 0x68, 0xa7, 0x44, 0x2d, 0x07, 0x0f, 0x1e, 0xd4, 0x0e, 0xe1, 0x72, 
  0xdc, 0x70, 0x43, 0x1f, 0x5c, 0x4e, 0x34, 0xba, 0xa0, 0xc4, 0xd5, 0xd5, 0x15, 0x57, 0xfe, 0x88, 
  0x08, 0x6b, 0x96, 0x7e, 0x8f, 0x1e, 0x3d, 0x10, 0x63, 0xcd, 0x72, 0x67, 0xef, 0xde, 0xbd, 0x00, 
  0x00, 0x67, 0x67, 0xe7, 0x86, 0x86, 0x06, 0x8c, 0xd9, 0x62, 0xeb, 0x81, 0x8c, 0x8e, 0x4d, 0x6d, 
  0xde, 0xbc, 0x19, 0x57, 0xfe, 0x28, 0x40, 0x08, 0x33, 0x33, 0x33, 0x2d, 0x59, 0x22, 0x17, 0x7c, 
  0x7d, 0x7d, 0x2f, 0x5f, 0xbe, 0x8c, 0x6b, 0x5a, 0x45, 0xa3, 0xbc, 0xf3, 0xce, 0x3b, 0x00, 0x80, 
  0xd0, 0xd0, 0x50, 0x53, 0x1b, 0x77, 0xb0, 0x04, 0x97, 0x13, 0x35, 0x73, 0x9c, 0x75, 0xc1, 0x1b, 
  0xcc, 0x11, 0x85, 0x2b, 0x57, 0xae, 0xe0, 0x6c, 0x1a, 0xc2, 0x5c, 0xbb, 0x76, 0xcd, 0xc2, 0xed, 
  0x43, 0x02, 0x6c, 0xf6, 0xef, 0xd8, 0xb1, 0xa3, 0xee, 0x9f, 0x12, 0x89, 0xc4, 0xf2, 0x3b, 0xb4, 
  0x9b, 0xb5, 0xa9, 0x0a, 0x5d, 0xa6, 0x4e, 0x9d, 0x8a, 0xb8, 0x0c, 0x99, 0xef, 0xe0, 0x72, 0xe2, 
  0xcb, 0x97, 0x2f, 0x75, 0x5f, 0xdd, 0x25, 0x25, 0x25, 0xe1, 0xca, 0x19, 0x1d, 0x01, 0x0d, 0x24, 
  0xde, 0xbc, 0x79, 0xd3, 0xf2, 0xed, 0x43, 0x02, 0x9c, 0x21, 0xee, 0x72, 0x72, 0x72, 0x9a, 0x36, 
  0x6d, 0xea, 0xe1, 0xe1, 0x91, 0x9c, 0x9c, 0x8c, 0x31, 0x5b, 0x44, 0xaa, 0xab, 0xab, 0xcd, 0xda, 
  0x9b, 0x92, 0x22, 0x3e, 0x3e, 0x3e, 0x96, 0x6f, 0x1f, 0x42, 0xe0, 0x0c, 0xb2, 0x19, 0x1a, 0x1a, 
  0xfa, 0xd7, 0x5f, 0x7f, 0x49, 0x24, 0x12, 0xd2, 0xc1, 0xfa, 0x8d, 0x72, 0xf4, 0xe8, 0x51, 0xa1, 
  0xcc, 0x2e, 0xea, 0xd3, 0xa7, 0x0f, 0x6d, 0x09, 0xd8, 0xc0, 0x69, 0x20, 0x00, 0x40, 0xf3, 0xe6, 
  0xcd, 0xf1, 0x66, 0x88, 0xce, 0xb6, 0x6d, 0xdb, 0x68, 0x15, 0x6d, 0x2e, 0x7a, 0xf7, 0x8b, 0x82, 
  0xc6, 0x72, 0xf3, 0x81, 0x88, 0x52, 0x54, 0x54, 0x24, 0xa0, 0x07, 0x78, 0x9c, 0xd3, 0x71, 0x68, 
  0x63, 0x25, 0x06, 0x4a, 0x4e, 0x4e, 0xa6, 0x2d, 0xc1, 0x0c, 0xcc, 0x5a, 0xc5, 0xcb, 0x73, 0xac, 
  0xc4, 0x40, 0xd4, 0x77, 0x59, 0x30, 0x0b, 0xd6, 0x9b, 0x28, 0xf2, 0x10, 0x6b, 0x30, 0xd0, 0xe9, 
  0xd3, 0xa7, 0xf3, 0xf2, 0xf2, 0x68, 0xab, 0x30, 0x83, 0x1b, 0x37, 0x6e, 0xd0, 0x96, 0x80, 0x0d, 
  0x6b, 0x30, 0x90, 0xe5, 0x47, 0x2c, 0x39, 0x72, 0xeb, 0xd6, 0x2d, 0xab, 0x59, 0x92, 0x2b, 0x78, 
  0x03, 0x3d, 0x7b, 0xf6, 0xec, 0xcc, 0x99, 0x33, 0xb4, 0x55, 0x98, 0x47, 0x7d, 0x7d, 0x3d, 0x95, 
  0x2d, 0x32, 0x48, 0x20, 0x78, 0x03, 0x7d, 0xfd, 0xf5, 0xd7, 0x42, 0x5c, 0x1f, 0xfd, 0xcb, 0x2f, 
  0xbf, 0xd0, 0x96, 0x80, 0x07, 0x6c, 0x53, 0x5a, 0xa9, 0x00, 0x21, 0x94, 0x4a, 0xa5, 0x75, 0x75, 
  0x75, 0xb4, 0x85, 0x98, 0x4d, 0xf3, 0xe6, 0xcd, 0x1f, 0x3e, 0x7c, 0xc8, 0x65, 0xfe, 0x2e, 0x4f, 
  0x10, 0x76, 0x0f, 0x74, 0xf6, 0xec, 0x59, 0x21, 0xba, 0x07, 0x00, 0x50, 0x52, 0x52, 0x62, 0x74, 
  0x39, 0x8a, 0xe0, 0x10, 0xb6, 0x81, 0xd6, 0xac, 0x59, 0x43, 0x5b, 0x02, 0x7b, 0x12, 0x13, 0x13, 
  0x69, 0x4b, 0xc0, 0x80, 0x80, 0x2f, 0x61, 0x37, 0x6f, 0xde, 0xec, 0xdc, 0xb9, 0x33, 0x6d, 0x15, 
  0xec, 0x71, 0x76, 0x76, 0x2e, 0x2f, 0x2f, 0x17, 0xca, 0x0b, 0x60, 0x53, 0x08, 0xb8, 0x07, 0xb2, 
  0xd8, 0x36, 0xc7, 0x84, 0xa8, 0xa9, 0xa9, 0x59, 0xbf, 0x7e, 0x3d, 0x6d, 0x15, 0x5c, 0x11, 0x6a, 
  0x0f, 0x54, 0x5c, 0x5c, 0x1c, 0x10, 0x10, 0x20, 0x50, 0xf1, 0x5a, 0xbc, 0xbc, 0xbc, 0x84, 0x1e, 
  0xa7, 0x56, 0xa8, 0x3d, 0xd0, 0xba, 0x75, 0xeb, 0x84, 0xee, 0x1e, 0x00, 0x40, 0x59, 0x59, 0x59, 
  0x52, 0x52, 0x12, 0x6d, 0x15, 0x9c, 0x10, 0x6a, 0x0f, 0xe4, 0xee, 0xee, 0x6e, 0x1d, 0x1b, 0x4c, 
  0xb5, 0x68, 0xd1, 0xa2, 0xb0, 0xb0, 0x10, 0xf3, 0x44, 0x77, 0x0b, 0x22, 0xc8, 0x1e, 0x68, 0xe7, 
  0xce, 0x9d, 0xd6, 0xe1, 0x1e, 0x00, 0x80, 0x42, 0xa1, 0xd8, 0xbf, 0x7f, 0x3f, 0x6d, 0x15, 0xec, 
  0x11, 0x5e, 0x0f, 0xa4, 0x54, 0x2a, 0xbd, 0xbc, 0xbc, 0xac, 0xc6, 0x40, 0x00, 0x00, 0x5f, 0x5f, 
  0xdf, 0xe2, 0xe2, 0x62, 0x8b, 0xad, 0xef, 0xc1, 0x8b, 0xf0, 0x44, 0xff, 0xf8, 0xe3, 0x8f, 0xd6, 
  0xe4, 0x1e, 0x00, 0xc0, 0xa3, 0x47, 0x8f, 0x56, 0xac, 0x58, 0x41, 0x5b, 0x05, 0x4b, 0x84, 0xd7, 
  0x03, 0x31, 0x84, 0x01, 0x11, 0x2e, 0x4e, 0x4e, 0x4e, 0x55, 0x55, 0x55, 0x28, 0xb1, 0xb6, 0xf8, 
  0x86, 0xc0, 0x7a, 0xa0, 0x94, 0x94, 0x14, 0xeb, 0x73, 0x0f, 0x00, 0xa0, 0xae, 0xae, 0xce, 0x30, 
  0x80, 0xa4, 0x20, 0x10, 0x58, 0x0f, 0xd4, 0xa7, 0x4f, 0x9f, 0x6b, 0xd7, 0xae, 0xd1, 0x56, 0x41, 
  0x8a, 0xcc, 0xcc, 0x4c, 0xc1, 0xc5, 0x37, 0x12, 0x92, 0x81, 0x1e, 0x3e, 0x7c, 0xe8, 0xef, 0xef, 
  0x4f, 0x5b, 0x05, 0x41, 0x06, 0x0d, 0x1a, 0x74, 0xfe, 0xfc, 0x79, 0xda, 0x2a, 0xcc, 0x43, 0x48, 
  0x97, 0xb0, 0x95, 0x2b, 0x57, 0xd2, 0x96, 0x40, 0x96, 0x5f, 0x7e, 0xf9, 0xe5, 0xc4, 0x89, 0x13, 
  0xb4, 0x55, 0x98, 0x87, 0x60, 0x7a, 0xa0, 0x07, 0x0f, 0x1e, 0x30, 0xc4, 0xee, 0xb7, 0x1a, 0xbc, 
  0xbd, 0xbd, 0x1f, 0x3f, 0x7e, 0x4c, 0x5b, 0x85, 0x19, 0x08, 0xa6, 0x07, 0xfa, 0xe9, 0xa7, 0x9f, 
  0x68, 0x4b, 0xb0, 0x04, 0xa5, 0xa5, 0xa5, 0x1b, 0x37, 0x6e, 0xa4, 0xad, 0xc2, 0x0c, 0x04, 0xd3, 
  0x03, 0x85, 0x85, 0x85, 0x65, 0x65, 0x65, 0xd1, 0x56, 0x61, 0x21, 0x8a, 0x8b, 0x8b, 0x85, 0x72, 
  0xb7, 0x27, 0x8c, 0x1e, 0x28, 0x39, 0x39, 0xd9, 0x76, 0xdc, 0x03, 0x00, 0x58, 0xbe, 0x7c, 0x39, 
  0x6d, 0x09, 0xa8, 0x08, 0xc3, 0x40, 0x07, 0x0f, 0x1e, 0xa4, 0x2d, 0xc1, 0xa2, 0x24, 0x24, 0x24, 
  0x08, 0x65, 0xd9, 0x86, 0x00, 0x2e, 0x61, 0xb5, 0xb5, 0xb5, 0x3e, 0x3e, 0x3e, 0x2c, 0xf6, 0xa5, 
  0x17, 0x34, 0xdd, 0xba, 0x75, 0x33, 0x1a, 0x23, 0x9c, 0x6f, 0x08, 0xa0, 0x07, 0x8a, 0x8f, 0x8f, 
  0xb7, 0x35, 0xf7, 0x00, 0x00, 0xd2, 0xd2, 0xd2, 0x04, 0xf1, 0xdc, 0x20, 0x80, 0x1e, 0x28, 0x24, 
  0x24, 0x24, 0x37, 0x37, 0x97, 0xb6, 0x0a, 0x0a, 0xb8, 0xb8, 0xb8, 0x54, 0x55, 0x55, 0xf1, 0x7c, 
  0xaa, 0x10, 0xdf, 0x7b, 0xa0, 0xfc, 0xfc, 0x7c, 0xdb, 0x74, 0x0f, 0x00, 0xe0, 0xc5, 0x8b, 0x17, 
  0xab, 0x57, 0xaf, 0xa6, 0xad, 0xa2, 0x11, 0xf8, 0x6e, 0x20, 0x01, 0xc5, 0xcd, 0x24, 0x41, 0x4c, 
  0x4c, 0x4c, 0x7e, 0x7e, 0x3e, 0x6d, 0x15, 0x4c, 0xf0, 0xdd, 0x40, 0x71, 0x71, 0x71, 0xb4, 0x25, 
  0x50, 0x66, 0xf1, 0xe2, 0xc5, 0xb4, 0x25, 0x30, 0x42, 0x21, 0x2e, 0x23, 0x32, 0x39, 0x39, 0x39, 
  0xb4, 0x9b, 0x87, 0x17, 0x5c, 0xbc, 0x78, 0x91, 0xf6, 0xa9, 0x30, 0x09, 0xaf, 0x7b, 0x20, 0xa1, 
  0xaf, 0x58, 0xc0, 0xc5, 0xc4, 0x89, 0x13, 0x69, 0x4b, 0x30, 0x09, 0xaf, 0x0d, 0x84, 0xb2, 0xcb, 
  0x98, 0x2d, 0x50, 0x51, 0x51, 0xb1, 0x7b, 0xf7, 0x6e, 0xda, 0x2a, 0x8c, 0xc3, 0xeb, 0xc7, 0xf8, 
  0xc0, 0xc0, 0x40, 0x6b, 0x8a, 0x06, 0xc7, 0x05, 0x17, 0x17, 0x97, 0xc2, 0xc2, 0x42, 0xbd, 0x4d, 
  0xd3, 0xf9, 0x00, 0x7f, 0x7b, 0xa0, 0xdb, 0xb7, 0x6f, 0x8b, 0xee, 0xd1, 0xf2, 0xe2, 0xc5, 0x8b, 
  0x55, 0xab, 0x56, 0xd1, 0x56, 0x61, 0x04, 0xfe, 0x1a, 0xe8, 0xf2, 0xe5, 0xcb, 0xb4, 0x25, 0xf0, 
  0x8b, 0xfd, 0xfb, 0xf7, 0x57, 0x56, 0x56, 0xd2, 0x56, 0xa1, 0x0f, 0x7f, 0x0d, 0x74, 0xeb, 0xd6, 
  0x2d, 0xda, 0x12, 0xf8, 0x85, 0x52, 0xa9, 0xdc, 0xbe, 0x7d, 0x3b, 0x6d, 0x15, 0xfa, 0xf0, 0xd7, 
  0x40, 0x85, 0x85, 0x85, 0xb4, 0x25, 0xb0, 0x81, 0xe8, 0x9b, 0x07, 0x1e, 0x86, 0x14, 0xe2, 0xaf, 
  0x81, 0x9e, 0x3c, 0x79, 0x42, 0x5b, 0x02, 0x1b, 0xc6, 0x8e, 0x1d, 0x3b, 0x64, 0xc8, 0x10, 0x42, 
  0x99, 0xe7, 0xe4, 0xe4, 0x14, 0x17, 0x17, 0x13, 0xca, 0x9c, 0x1d, 0x3c, 0x35, 0x50, 0x7d, 0x7d, 
  0x3d, 0xfa, 0xf2, 0xd3, 0xe0, 0xe0, 0xe0, 0x66, 0xcd, 0x9a, 0x11, 0xd5, 0x83, 0x4e, 0xb7, 0x6e, 
  0xdd, 0x46, 0x8d, 0x1a, 0x45, 0x2e, 0x7f, 0xbe, 0x05, 0xc6, 0xe3, 0xa9, 0x81, 0x6a, 0x6b, 0x6b, 
  0xd1, 0x83, 0x1f, 0x0e, 0x1a, 0x34, 0x68, 0xe0, 0xc0, 0x81, 0x44, 0xf5, 0xa0, 0x23, 0x95, 0x4a, 
  0x89, 0x1a, 0xe8, 0xd7, 0x5f, 0x7f, 0x25, 0x97, 0x39, 0x0b, 0x78, 0x6a, 0x20, 0x27, 0x27, 0x27, 
  0x07, 0x07, 0xd4, 0x9d, 0x84, 0x5a, 0xb6, 0x6c, 0x39, 0x6d, 0xda, 0x34, 0xa2, 0x7a, 0xd0, 0xa9, 
  0xaa, 0xaa, 0xf2, 0xf7, 0xf7, 0x27, 0xb7, 0x0b, 0x33, 0xdf, 0xde, 0xad, 0xf2, 0xd7, 0x40, 0xee, 
  0xee, 0xee, 0x88, 0x07, 0x2b, 0x95, 0xca, 0x01, 0x03, 0x06, 0x90, 0x94, 0x63, 0x06, 0xcf, 0x9f, 
  0x3f, 0x07, 0x00, 0xcc, 0x9a, 0x35, 0x8b, 0x50, 0xfe, 0x7c, 0x1b, 0xf8, 0xe5, 0xa9, 0x81, 0x00, 
  0x00, 0xe8, 0xab, 0xc0, 0x2a, 0x2a, 0x2a, 0x9a, 0x35, 0x6b, 0xe6, 0xeb, 0xeb, 0xcb, 0xbd, 0x50, 
  0xee, 0x81, 0x9b, 0xd5, 0x6a, 0x35, 0x00, 0x60, 0xdc, 0xb8, 0x71, 0xdc, 0xc5, 0x18, 0xa5, 0x77, 
  0xef, 0xde, 0x84, 0x72, 0x66, 0x07, 0x7f, 0x0d, 0x14, 0x19, 0x19, 0x89, 0x78, 0xa4, 0x66, 0xc2, 
  0xeb, 0xec, 0xd9, 0xb3, 0xb9, 0x17, 0x6a, 0x6f, 0x6f, 0x1f, 0x18, 0x18, 0xc8, 0x25, 0x54, 0x4f, 
  0x43, 0x43, 0x03, 0x00, 0xa0, 0x6d, 0xdb, 0xb6, 0x84, 0x96, 0x41, 0xf2, 0x6d, 0x3c, 0x9a, 0xbf, 
  0x06, 0x9a, 0x3f, 0x7f, 0x3e, 0xe2, 0x98, 0x8a, 0x26, 0x2a, 0xca, 0xe2, 0xc5, 0x8b, 0xb9, 0xf7, 
  0x1f, 0x2a, 0x95, 0xaa, 0x6d, 0xdb, 0xb6, 0x5c, 0x5e, 0x39, 0x69, 0xa7, 0x6f, 0xcf, 0x98, 0x31, 
  0x83, 0xa3, 0x18, 0x43, 0x12, 0x12, 0x12, 0x5a, 0xb4, 0x68, 0x81, 0x3d, 0x5b, 0x2e, 0xf0, 0xd7, 
  0x40, 0xee, 0xee, 0xee, 0x88, 0xcb, 0xa3, 0x64, 0x32, 0x19, 0x00, 0xc0, 0xd7, 0xd7, 0xb7, 0x5f, 
  0xbf, 0x7e, 0xdc, 0xcb, 0x0d, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x63, 0x9d, 0x5c, 0x3b, 0x7c, 0xf5, 
  0xde, 0x7b, 0xef, 0x61, 0x8c, 0xf7, 0x33, 0x78, 0xf0, 0xe0, 0xa4, 0xa4, 0xa4, 0xb7, 0xdf, 0x7e, 
  0x1b, 0x57, 0x86, 0xb8, 0xe0, 0xaf, 0x81, 0x00, 0x00, 0x31, 0x31, 0x31, 0x28, 0xcf, 0xe7, 0x81, 
  0x81, 0x81, 0x9a, 0x0f, 0x63, 0xc6, 0x8c, 0xe1, 0x5e, 0xa8, 0x4c, 0x26, 0xeb, 0xd1, 0xa3, 0x07, 
  0xeb, 0xe4, 0xe5, 0xe5, 0xe5, 0x9a, 0x0f, 0x3e, 0x3e, 0x3e, 0xdc, 0x6f, 0xed, 0x3d, 0x3d, 0x3d, 
  0xe7, 0xce, 0x9d, 0x9b, 0x9a, 0x9a, 0x7a, 0xee, 0xdc, 0xb9, 0x11, 0x23, 0x46, 0x70, 0xcc, 0x8d, 
  0x08, 0xb4, 0x67, 0xb4, 0x35, 0x0e, 0x73, 0xe4, 0x25, 0x4d, 0xb8, 0x56, 0xcd, 0x91, 0xcf, 0x9f, 
  0x3f, 0x77, 0x75, 0x75, 0xe5, 0xd8, 0x20, 0x0b, 0x16, 0x2c, 0x48, 0x49, 0x49, 0x61, 0x9d, 0x3c, 
  0x30, 0x30, 0x50, 0xad, 0x56, 0x6b, 0xf4, 0x1c, 0x3a, 0x74, 0x88, 0x75, 0x26, 0xd1, 0xd1, 0xd1, 
  0xd7, 0xae, 0x5d, 0xab, 0xae, 0xae, 0xa6, 0xd7, 0xf0, 0x48, 0x08, 0xc0, 0x40, 0x10, 0xc2, 0x2b, 
  0x57, 0xae, 0xcc, 0x9f, 0x3f, 0x3f, 0x24, 0x24, 0x44, 0xaf, 0xa1, 0x3b, 0x76, 0xec, 0x78, 0xe7, 
  0xce, 0x1d, 0xdd, 0x23, 0x97, 0x2e, 0x5d, 0xca, 0xfa, 0xdc, 0x6b, 0x68, 0xd3, 0xa6, 0x8d, 0x42, 
  0xa1, 0x60, 0xbd, 0xfd, 0xb4, 0x44, 0x22, 0x79, 0xf0, 0xe0, 0x81, 0x46, 0x8c, 0x52, 0xa9, 0x34, 
  0x6b, 0x89, 0x7b, 0xdf, 0xbe, 0x7d, 0x97, 0x2f, 0x5f, 0x7e, 0xe1, 0xc2, 0x85, 0x86, 0x86, 0x06, 
  0x1a, 0xcd, 0xcc, 0x06, 0x61, 0x18, 0x48, 0x4b, 0x7a, 0x7a, 0x7a, 0x7c, 0x7c, 0xfc, 0x97, 0x5f, 
  0x7e, 0xb9, 0x66, 0xcd, 0x9a, 0x53, 0xa7, 0x4e, 0x19, 0x1e, 0x50, 0x54, 0x54, 0xc4, 0xee, 0xc4, 
  0xeb, 0x52, 0x54, 0x54, 0xf4, 0xee, 0xbb, 0xef, 0xb2, 0x4e, 0x9e, 0x92, 0x92, 0xa2, 0xd5, 0x83, 
  0xb2, 0x1f, 0x43, 0x58, 0x58, 0xd8, 0x8a, 0x15, 0x2b, 0x72, 0x72, 0x72, 0x2c, 0xd7, 0x8e, 0xf8, 
  0x10, 0x98, 0x81, 0x50, 0xe8, 0xd9, 0xb3, 0x27, 0xeb, 0x73, 0xaf, 0x21, 0x39, 0x39, 0xf9, 0xfa, 
  0xf5, 0xeb, 0xac, 0x93, 0xef, 0xdf, 0xbf, 0x5f, 0x2b, 0x86, 0x79, 0x6b, 0xcb, 0xb1, 0x63, 0xc7, 
  0x5e, 0xbf, 0x7e, 0x9d, 0x5e, 0x53, 0x61, 0xc0, 0x0a, 0x0d, 0xc4, 0x7d, 0x49, 0xf9, 0xb8, 0x71, 
  0xe3, 0x20, 0x84, 0xac, 0x1f, 0x98, 0x57, 0xae, 0x5c, 0xa9, 0xab, 0xc7, 0xe8, 0xf3, 0xfc, 0xa8, 
  0x51, 0xa3, 0xae, 0x5e, 0xbd, 0x4a, 0xa9, 0x85, 0x70, 0x62, 0x85, 0x06, 0x82, 0x10, 0x76, 0xef, 
  0xde, 0x9d, 0x8b, 0x81, 0x1c, 0x1c, 0x1c, 0x20, 0x84, 0xdb, 0xb6, 0x6d, 0x63, 0x97, 0x3c, 0x32, 
  0x32, 0x52, 0x57, 0x4c, 0x49, 0x49, 0x89, 0xde, 0x80, 0xd6, 0xa7, 0x9f, 0x7e, 0x4a, 0xa9, 0x61, 
  0xf0, 0x63, 0x9d, 0x06, 0x3a, 0x72, 0xe4, 0x08, 0x17, 0x03, 0x01, 0x00, 0x4a, 0x4a, 0x4a, 0x34, 
  0x6f, 0xb5, 0x58, 0xe0, 0xe0, 0xe0, 0x50, 0x5e, 0x5e, 0xae, 0xab, 0x47, 0x77, 0x5d, 0xce, 0x8e, 
  0x1d, 0x3b, 0x68, 0x35, 0x0b, 0x09, 0xac, 0xd3, 0x40, 0x6a, 0xb5, 0xba, 0x6d, 0xdb, 0xb6, 0x5c, 
  0x0c, 0x14, 0x1d, 0x1d, 0x0d, 0x21, 0x44, 0x7f, 0x9d, 0xa2, 0x87, 0xee, 0x6d, 0x10, 0x84, 0x30, 
  0x3b, 0x3b, 0x5b, 0xf3, 0xfd, 0xc2, 0x85, 0x0b, 0xe9, 0xb4, 0x08, 0x31, 0xac, 0xd3, 0x40, 0x10, 
  0x42, 0x8e, 0x9b, 0xc9, 0xfb, 0xf8, 0xf8, 0x40, 0x08, 0xef, 0xdd, 0xbb, 0xc7, 0x2e, 0xf9, 0xf8, 
  0xf1, 0xe3, 0xf5, 0xf4, 0x44, 0x45, 0x45, 0x01, 0x00, 0x6a, 0x6a, 0x6a, 0x68, 0x34, 0x06, 0x41, 
  0xac, 0xd6, 0x40, 0x2a, 0x95, 0x8a, 0xe3, 0x2a, 0x2a, 0xcd, 0xd3, 0xb8, 0x5c, 0x2e, 0x67, 0x91, 
  0xd6, 0xd7, 0xd7, 0x57, 0x4f, 0x4f, 0x52, 0x52, 0x52, 0xa7, 0x4e, 0x9d, 0x28, 0x34, 0x04, 0x61, 
  0xac, 0xd6, 0x40, 0x10, 0x42, 0x8e, 0x2b, 0xa3, 0xa7, 0x4f, 0x9f, 0x0e, 0x21, 0xbc, 0x7a, 0xf5, 
  0x2a, 0xbb, 0xe4, 0x99, 0x99, 0x99, 0xba, 0x62, 0xd4, 0x6a, 0xf5, 0xdf, 0x7f, 0xff, 0x4d, 0xa9, 
  0x25, 0x08, 0x62, 0xcd, 0x06, 0x82, 0x10, 0xb6, 0x6b, 0xd7, 0x8e, 0xb5, 0x81, 0x3c, 0x3d, 0x3d, 
  0x95, 0x4a, 0x25, 0x84, 0xb0, 0x4f, 0x9f, 0x3e, 0x2c, 0x92, 0x2f, 0x5d, 0xba, 0x94, 0x76, 0xed, 
  0x2d, 0x81, 0x95, 0x1b, 0x88, 0xe3, 0x0c, 0xe2, 0x7d, 0xfb, 0xf6, 0x41, 0xb6, 0x9d, 0x90, 0x9b, 
  0x9b, 0x5b, 0x5d, 0x5d, 0x1d, 0xed, 0x06, 0x20, 0x8e, 0x95, 0x1b, 0x08, 0x42, 0xc8, 0x65, 0x72, 
  0xe0, 0x1b, 0x6f, 0xbc, 0xa1, 0xc9, 0x64, 0xce, 0x9c, 0x39, 0x2c, 0x92, 0x9f, 0x3e, 0x7d, 0x9a, 
  0x6e, 0xdd, 0x2d, 0x80, 0xf5, 0x1b, 0xe8, 0xc1, 0x83, 0x07, 0xac, 0x0d, 0x04, 0x00, 0xc8, 0xcd, 
  0xcd, 0x85, 0x10, 0x56, 0x57, 0x57, 0xb3, 0x58, 0x31, 0xa8, 0xb9, 0x8b, 0xb2, 0x6e, 0xac, 0xdf, 
  0x40, 0x90, 0xc3, 0x98, 0x32, 0x00, 0x60, 0xf4, 0xe8, 0xd1, 0x9a, 0x4c, 0x58, 0xec, 0x6c, 0x6a, 
  0x6f, 0x6f, 0x5f, 0x5a, 0x5a, 0x4a, 0xb5, 0xea, 0xc4, 0xb1, 0x09, 0x03, 0x41, 0x08, 0xb9, 0xcc, 
  0x50, 0xd6, 0xbe, 0x27, 0xef, 0xd5, 0xab, 0x97, 0xb9, 0x69, 0x3f, 0xf8, 0xe0, 0x03, 0xaa, 0xf5, 
  0x26, 0x8e, 0xad, 0x18, 0xe8, 0xf7, 0xdf, 0x7f, 0x67, 0x6d, 0xa0, 0xc5, 0x8b, 0x17, 0x6b, 0x32, 
  0xc9, 0xcf, 0xcf, 0x37, 0x77, 0x96, 0xaa, 0x4c, 0x26, 0xa3, 0x5b, 0x71, 0xd2, 0xd8, 0x8a, 0x81, 
  0x20, 0x84, 0xb1, 0xb1, 0xb1, 0xec, 0x0c, 0x64, 0x6f, 0x6f, 0xaf, 0x1d, 0xc2, 0x61, 0xb1, 0x9f, 
  0xd7, 0xa1, 0x43, 0x87, 0xe8, 0x56, 0x9c, 0x28, 0x36, 0x64, 0x20, 0x08, 0x61, 0xe7, 0xce, 0x9d, 
  0xd9, 0x79, 0x68, 0xd8, 0xb0, 0x61, 0xda, 0x4c, 0xcc, 0x5d, 0x34, 0xe8, 0xe9, 0xe9, 0xa9, 0x52, 
  0xa9, 0x28, 0xd6, 0x9a, 0x28, 0xb6, 0x65, 0x20, 0x2e, 0x4f, 0x64, 0xf9, 0xf9, 0xf9, 0xda, 0x7c, 
  0xcc, 0xdd, 0xd9, 0x34, 0x21, 0x21, 0x81, 0x62, 0xad, 0x89, 0x62, 0x5b, 0x06, 0x82, 0x10, 0xb2, 
  0xde, 0xf8, 0xa7, 0x67, 0xcf, 0x9e, 0xda, 0x4c, 0x72, 0x73, 0x73, 0xcd, 0xba, 0x19, 0x6a, 0xd7, 
  0xae, 0x1d, 0xc5, 0x2a, 0x13, 0xc5, 0xe6, 0x0c, 0x04, 0x21, 0x5c, 0xb2, 0x64, 0x09, 0x3b, 0x0f, 
  0x9d, 0x39, 0x73, 0x46, 0x9b, 0xc9, 0xa5, 0x4b, 0x97, 0xcc, 0x4a, 0x1b, 0x17, 0x17, 0x47, 0xb1, 
  0xca, 0xe4, 0xb0, 0x45, 0x03, 0x41, 0xb6, 0x13, 0x7d, 0x5a, 0xb5, 0x6a, 0xa5, 0x5d, 0xb2, 0x03, 
  0x21, 0xdc, 0xb7, 0x6f, 0x1f, 0x7a, 0xda, 0xa0, 0xa0, 0x20, 0x8a, 0xf5, 0x25, 0x87, 0x8d, 0x1a, 
  0x48, 0xa9, 0x54, 0xb2, 0x9b, 0xf2, 0xac, 0x37, 0xae, 0x63, 0xd6, 0xd6, 0x82, 0x1b, 0x37, 0x6e, 
  0xa4, 0x54, 0x5d, 0x82, 0xd8, 0xa8, 0x81, 0x20, 0x84, 0x65, 0x65, 0x65, 0xec, 0xe2, 0x9a, 0x69, 
  0x97, 0x7d, 0x69, 0xf8, 0xec, 0xb3, 0xcf, 0xd0, 0xd3, 0x5a, 0xdf, 0xeb, 0x55, 0xdb, 0x35, 0x10, 
  0x84, 0xf0, 0xc6, 0x8d, 0x1b, 0xce, 0xce, 0xce, 0xe6, 0x1a, 0xa8, 0x4b, 0x97, 0x2e, 0x7a, 0xf9, 
  0x7c, 0xf2, 0xc9, 0x27, 0x88, 0x69, 0xe7, 0xcf, 0x9f, 0x4f, 0xa5, 0xa6, 0xe4, 0xb0, 0x69, 0x03, 
  0x41, 0x08, 0x33, 0x32, 0x32, 0xd0, 0x43, 0xa1, 0x69, 0x59, 0xbf, 0x7e, 0xbd, 0x5e, 0x3e, 0xd1, 
  0xd1, 0xd1, 0x88, 0x69, 0x75, 0x97, 0x1d, 0x5a, 0x01, 0xb6, 0x6e, 0x20, 0x08, 0xe1, 0x8d, 0x1b, 
  0x37, 0x58, 0xbc, 0x69, 0xd7, 0x1d, 0x16, 0xd2, 0x80, 0xb8, 0xdf, 0x7b, 0x70, 0x70, 0x30, 0x95, 
  0x6a, 0x12, 0x42, 0x34, 0x10, 0x84, 0x10, 0xde, 0xb9, 0x73, 0xc7, 0xcd, 0xcd, 0xcd, 0x2c, 0x03, 
  0x19, 0x1d, 0xda, 0x41, 0x5c, 0x4e, 0xb4, 0x6a, 0xd5, 0x2a, 0xcb, 0xd7, 0x91, 0x10, 0xa2, 0x81, 
  0xfe, 0xe5, 0xee, 0xdd, 0xbb, 0x01, 0x01, 0x01, 0x66, 0x79, 0xc8, 0xe8, 0xa4, 0xd5, 0xa4, 0xa4, 
  0x24, 0x94, 0x08, 0x21, 0x56, 0x33, 0x3f, 0x5a, 0x34, 0xd0, 0xff, 0xa9, 0xac, 0xac, 0x34, 0x37, 
  0xba, 0xaa, 0xd1, 0x95, 0xed, 0x79, 0x79, 0x79, 0xc1, 0xc1, 0xc1, 0xcc, 0x09, 0xdf, 0x7a, 0xeb, 
  0x2d, 0xcb, 0x57, 0x90, 0x04, 0xa2, 0x81, 0xfe, 0x43, 0x5d, 0x5d, 0xdd, 0xbc, 0x79, 0xf3, 0xd0, 
  0x0d, 0xd4, 0xb4, 0x69, 0xd3, 0x82, 0x82, 0x02, 0xc3, 0x7c, 0x94, 0x4a, 0x65, 0xa3, 0x31, 0x1b, 
  0x37, 0x6c, 0xd8, 0x60, 0xf9, 0x0a, 0x62, 0x47, 0x34, 0x90, 0x11, 0xd6, 0xad, 0x5b, 0x87, 0xee, 
  0xa1, 0xf0, 0xf0, 0x70, 0x53, 0xf9, 0x6c, 0xda, 0xb4, 0x49, 0x2a, 0x95, 0x9a, 0x4a, 0xe8, 0xe0, 
  0xe0, 0x70, 0xff, 0xfe, 0x7d, 0x4b, 0xd6, 0x8b, 0x04, 0xa2, 0x81, 0x8c, 0x73, 0xfd, 0xfa, 0xf5, 
  0x88, 0x88, 0x08, 0x44, 0x0f, 0x4d, 0x99, 0x32, 0xc5, 0x54, 0x3e, 0x79, 0x79, 0x79, 0x0c, 0xe1, 
  0x66, 0xac, 0xe0, 0x89, 0x4c, 0x34, 0x10, 0x13, 0xe8, 0x31, 0x75, 0x97, 0x2d, 0x5b, 0xc6, 0x90, 
  0xcf, 0xe6, 0xcd, 0x9b, 0x4d, 0x25, 0x14, 0xfa, 0x9c, 0x57, 0xd1, 0x40, 0x8d, 0x70, 0xf5, 0xea, 
  0x55, 0xc4, 0xdd, 0x77, 0x98, 0xef, 0x69, 0x32, 0x32, 0x32, 0x34, 0xcb, 0xe3, 0x0d, 0x39, 0x79, 
  0xf2, 0xa4, 0xc5, 0xaa, 0x83, 0x1d, 0xd1, 0x40, 0x48, 0x1c, 0x38, 0x70, 0x20, 0x28, 0x28, 0xa8, 
  0x51, 0x0f, 0x35, 0x3a, 0x71, 0x2c, 0x3e, 0x3e, 0xde, 0xf0, 0x41, 0xcf, 0xc9, 0xc9, 0xc9, 0xe8, 
  0x9d, 0xb8, 0x20, 0x10, 0x0d, 0x64, 0x06, 0xbb, 0x77, 0xef, 0xf6, 0xf4, 0xf4, 0x64, 0xf6, 0x10, 
  0xca, 0x1e, 0xef, 0x5b, 0xb7, 0x6e, 0x75, 0x74, 0x74, 0xd4, 0x4d, 0x25, 0x97, 0xcb, 0x2d, 0xa0, 
  0x9f, 0x04, 0xa2, 0x81, 0xcc, 0xe3, 0xc9, 0x93, 0x27, 0x1b, 0x37, 0x6e, 0x64, 0x18, 0x2e, 0xb2, 
  0xb7, 0xb7, 0x47, 0x09, 0x7b, 0x58, 0x50, 0x50, 0xb0, 0x7a, 0xf5, 0x6a, 0x0f, 0x0f, 0x0f, 0x6d, 
  0xc2, 0x19, 0x33, 0x66, 0x58, 0x40, 0x3f, 0x76, 0x44, 0x03, 0xb1, 0x41, 0xa9, 0x54, 0xae, 0x59, 
  0xb3, 0xc6, 0x94, 0x8d, 0xa4, 0x52, 0xa9, 0x5e, 0x68, 0x0e, 0x53, 0x94, 0x96, 0x96, 0xbe, 0xff, 
  0xfe, 0xfb, 0x7e, 0x7e, 0x7e, 0x9a, 0x84, 0xb1, 0xb1, 0xb1, 0xa4, 0x95, 0x63, 0x47, 0x34, 0x10, 
  0x27, 0x12, 0x13, 0x13, 0x3b, 0x75, 0xea, 0x64, 0xe8, 0xa1, 0x26, 0x4d, 0x9a, 0x3c, 0x7d, 0xfa, 
  0x14, 0x31, 0x93, 0xda, 0xda, 0xda, 0xb8, 0xb8, 0x38, 0xcd, 0x04, 0xb7, 0xd5, 0xab, 0x57, 0x13, 
  0x15, 0x8c, 0x1d, 0xd1, 0x40, 0x18, 0x38, 0x7f, 0xfe, 0xfc, 0x47, 0x1f, 0x7d, 0xe4, 0xed, 0xed, 
  0xad, 0xeb, 0xa1, 0xe0, 0xe0, 0xe0, 0xc7, 0x8f, 0x1f, 0xa3, 0x67, 0x52, 0x5b, 0x5b, 0xbb, 0x6f, 
  0xdf, 0xbe, 0xc8, 0xc8, 0xc8, 0xf3, 0xe7, 0xcf, 0xeb, 0x4e, 0x9c, 0xe5, 0x39, 0x76, 0x90, 0x67, 
  0x1b, 0x98, 0x09, 0x97, 0xea, 0xea, 0xea, 0xa3, 0x47, 0x8f, 0x5e, 0xb9, 0x72, 0xe5, 0xd7, 0x5f, 
  0x7f, 0x2d, 0x29, 0x29, 0x01, 0x00, 0x78, 0x79, 0x79, 0xa5, 0xa5, 0xa5, 0xb5, 0x6e, 0xdd, 0xda, 
  0xac, 0x7c, 0xb2, 0xb3, 0xb3, 0x43, 0x43, 0x43, 0xb9, 0x6c, 0x39, 0x65, 0x49, 0x44, 0x03, 0xe1, 
  0xa7, 0xa6, 0xa6, 0xe6, 0xc4, 0x89, 0x13, 0xc7, 0x8e, 0x1d, 0x3b, 0x79, 0xf2, 0x64, 0x40, 0x40, 
  0xc0, 0xed, 0xdb, 0xb7, 0x75, 0x6f, 0x96, 0xad, 0x0c, 0xd1, 0x40, 0x04, 0x79, 0xfe, 0xfc, 0xf9, 
  0xcd, 0x9b, 0x37, 0x3b, 0x74, 0xe8, 0x20, 0x1a, 0x48, 0x44, 0xc4, 0x38, 0xc2, 0xb8, 0xd0, 0x8a, 
  0xf0, 0x16, 0xd1, 0x40, 0x22, 0x9c, 0x10, 0x0d, 0x24, 0xc2, 0x09, 0xd1, 0x40, 0x22, 0x9c, 0xf8, 
  0x1f, 0x0a, 0x69, 0x45, 0x22, 0xfc, 0x8e, 0x80, 0x06, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 
  0x44, 0xae, 0x42, 0x60, 0x82
        };
        server.send_P(200, "image/png", (const char*)pwa_icon_png, sizeof(pwa_icon_png));
    });
    
    // Handle manifest.json requests - serve PWA manifest from PROGMEM
    server.on("/manifest.json", HTTP_GET, [](){
        Serial.println("Manifest request received - serving PWA manifest from PROGMEM");
        server.send_P(200, "application/json", manifest_json);
    });
    
    // System status endpoint
    server.on("/status", HTTP_GET, [](){
        String stateStr;
        switch(currentState) {
            case OFF: stateStr = "OFF"; break;
            case ACCESSORY: stateStr = "ACCESSORY"; break;
            case IGNITION: stateStr = "IGNITION"; break;
            case RUNNING: stateStr = "RUNNING"; break;
            case CONFIG_MODE: stateStr = "CONFIG MODE"; break;
            default: stateStr = "UNKNOWN"; break;
        }
        
        String json = "{";
        json += "\"state\":\"" + stateStr + "\",";
        json += "\"bluetooth\":" + String((bluetoothEnabled && bluetoothAuthenticated) ? "true" : "false") + ",";
        json += "\"bluetoothEnabled\":" + String(bluetoothEnabled ? "true" : "false") + ",";
        json += "\"starterPulse\":" + String(starterPulseTime) + ",";
        json += "\"autoLockTimeout\":" + String(autoLockTimeout);
        json += "}";
        
        server.send(200, "application/json", json);
    });
    
    // Bluetooth devices endpoint
    server.on("/devices", HTTP_GET, [](){
        Serial.println("Bluetooth devices request received");
        if (!bluetoothEnabled || !bluetoothInitialized) {
            server.send(200, "application/json", "[]");
            return;
        }
        const char* json = getDevicesJson();
        server.send(200, "application/json", json);
    });
    
    // Bluetooth pairing toggle endpoint
    server.on("/pairing", HTTP_GET, [](){
        Serial.println("Bluetooth pairing toggle request received");
        if (!bluetoothEnabled || !bluetoothInitialized) {
            server.send(400, "text/plain", "Bluetooth is disabled");
            return;
        }
        toggleBluetoothPairingMode();
        String status = isPairingMode ? "Pairing mode active" : "Pairing mode inactive";
        server.send(200, "text/plain", status);
    });
    
    // Set device name endpoint
    server.on("/setname", HTTP_POST, [](){
        Serial.println("Set device name request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                const char* macStr = doc["mac"];
                const char* name = doc["name"];
                
                if (macStr && name) {
                    esp_bd_addr_t address;
                    int parsed = sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &address[0], &address[1], &address[2],
                        &address[3], &address[4], &address[5]);
                    
                    if (parsed == 6) {
                        saveDeviceName(address, name);
                        invalidateDeviceCache();
                        server.send(200, "text/plain", "Device name updated");
                        return;
                    }
                }
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });
    
    // Remove device endpoint
    server.on("/remove", HTTP_POST, [](){
        Serial.println("Remove device request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                const char* macStr = doc["mac"];
                
                if (macStr) {
                    esp_bd_addr_t address;
                    int parsed = sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &address[0], &address[1], &address[2],
                        &address[3], &address[4], &address[5]);
                    
                    if (parsed == 6) {
                        esp_err_t err = esp_ble_remove_bond_device(address);
                        if (err == ESP_OK) {
                            invalidateDeviceCache();
                            server.send(200, "text/plain", "Device removed");
                        } else {
                            server.send(500, "text/plain", "Failed to remove device");
                        }
                        return;
                    }
                }
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });
    
    // Priority toggle endpoint (placeholder for now)
    server.on("/priority", HTTP_POST, [](){
        Serial.println("Priority toggle request received");
        server.send(200, "text/plain", "Priority feature not implemented yet");
    });

    // Route for updating starter pulse time
    server.on("/update_pulse", HTTP_POST, [](){
        if (server.hasArg("pulse_time")) {
            String pulseTimeStr = server.arg("pulse_time");
            unsigned long newPulseTime = pulseTimeStr.toInt();
            
            // Validate the input
            if (newPulseTime >= 100 && newPulseTime <= 3000) {
                starterPulseTime = newPulseTime;
                preferences.putULong("starter_pulse", starterPulseTime);
                DEBUG_PRINT("Starter pulse time updated to: ");
                DEBUG_PRINT(starterPulseTime);
                DEBUG_PRINTLN("ms");
                server.send(200, "text/plain", "Starter crank time updated successfully");
            } else {
                server.send(400, "text/plain", "Invalid crank time. Must be between 100ms and 3000ms");
            }
        } else {
            server.send(400, "text/plain", "Missing pulse_time parameter");
        }
    });

    // Route for updating auto-lock timeout
    server.on("/update_autolock", HTTP_POST, [](){
        if (server.hasArg("auto_lock")) {
            String timeoutStr = server.arg("auto_lock");
            unsigned long newTimeout = timeoutStr.toInt();
            
            // Validate the input
            if (newTimeout >= 5000 && newTimeout <= 120000) {
                autoLockTimeout = newTimeout;
                preferences.putULong("auto_lock_timeout", autoLockTimeout);
                DEBUG_PRINT("Auto-lock timeout updated to: ");
                DEBUG_PRINT(autoLockTimeout);
                DEBUG_PRINTLN("ms");
                server.send(200, "text/plain", "Auto-lock timeout updated successfully");
            } else {
                server.send(400, "text/plain", "Invalid timeout. Must be between 5000ms and 120000ms");
            }
        } else {
            server.send(400, "text/plain", "Missing auto_lock parameter");
        }
    });



    // WiFi password management endpoints
    server.on("/wifi_password", HTTP_GET, [](){
        Serial.println("WiFi password request received");
        String json = "{\"password\":\"" + ap_password + "\"}";
        server.send(200, "application/json", json);
    });
    
    // Web interface password endpoints
    server.on("/web_password", HTTP_GET, [](){
        Serial.println("Web password request received");
        String json = "{\"password\":\"" + web_password + "\"}";
        server.send(200, "application/json", json);
    });
    
    server.on("/validate_web_password", HTTP_POST, [](){
        Serial.println("Web password validation request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                const char* password = doc["password"];
                
                if (password && String(password) == web_password) {
                    server.send(200, "text/plain", "Password correct");
                    return;
                } else {
                    server.send(401, "text/plain", "Invalid password");
                    return;
                }
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });
    
    server.on("/update_web_password", HTTP_POST, [](){
        Serial.println("Web password update request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                const char* newPassword = doc["password"];
                
                if (newPassword && strlen(newPassword) >= 4) {
                    web_password = String(newPassword);
                    preferences.putString("web_password", web_password);
                    Serial.printf("Web password updated: %s\n", web_password.c_str());
                    
                    server.send(200, "text/plain", "Web interface password updated successfully");
                    return;
                }
            }
        }
        server.send(400, "text/plain", "Invalid request or password too short");
    });
    
    server.on("/update_wifi_password", HTTP_POST, [](){
        Serial.println("WiFi password update request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                const char* newPassword = doc["password"];
                
                if (newPassword && strlen(newPassword) >= 8) {
                    ap_password = String(newPassword);
                    preferences.putString("wifi_password", ap_password);
                    Serial.printf("WiFi password saved (will apply on exit): %s\n", ap_password.c_str());
                    
                    server.send(200, "text/plain", "WiFi password saved successfully");
                    return;
                }
            }
        }
        server.send(400, "text/plain", "Invalid request or password too short");
    });

    // RFID endpoints
    server.on("/rfid_keys", HTTP_GET, [](){
        Serial.println("RFID keys request received");
        String json = getRfidKeysJson();
        server.send(200, "application/json", json);
    });
    
    server.on("/rfid_pair", HTTP_POST, [](){
        Serial.println("RFID pair mode request received");
        rfidPairingMode = true;
        Serial.println("RFID: Pairing mode activated - scan a tag to pair");
        server.send(200, "text/plain", "RFID pairing mode activated");
    });
    
    server.on("/rfid_remove", HTTP_POST, [](){
        Serial.println("RFID remove request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                int index = doc["index"];
                
                if (removeRfidKey(index)) {
                    server.send(200, "text/plain", "RFID key removed");
                } else {
                    server.send(400, "text/plain", "Invalid key index");
                }
                return;
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });
    
    server.on("/rfid_status", HTTP_GET, [](){
        String json = "{";
        json += "\"pairing\":" + String(rfidPairingMode ? "true" : "false") + ",";
        json += "\"authenticated\":" + String(rfidAuthenticated ? "true" : "false") + ",";
        json += "\"count\":" + String(numStoredKeys) + ",";
        json += "\"max\":" + String(MAX_RFID_KEYS);
        json += "}";
        server.send(200, "application/json", json);
    });

    // Bluetooth enable/disable endpoints
    server.on("/bluetooth_status", HTTP_GET, [](){
        Serial.println("Bluetooth status request received");
        String json = "{";
        json += "\"enabled\":" + String(bluetoothEnabled ? "true" : "false") + ",";
        json += "\"initialized\":" + String(bluetoothInitialized ? "true" : "false");
        json += "}";
        server.send(200, "application/json", json);
    });
    
    server.on("/bluetooth_toggle", HTTP_POST, [](){
        Serial.println("Bluetooth toggle request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                bool newState = doc["enabled"];
                
                // Apply the change immediately
                if (newState && !bluetoothEnabled) {
                    // Enabling Bluetooth
                                         bluetoothEnabled = true;
                     preferences.putBool("bt_enabled", bluetoothEnabled);
                    
                    if (!bluetoothInitialized) {
                        restartBluetooth();
                    }
                    
                    Serial.println("Bluetooth enabled and started immediately");
                    server.send(200, "text/plain", "Bluetooth enabled successfully");
                    
                } else if (!newState && bluetoothEnabled) {
                    // Disabling Bluetooth
                                         bluetoothEnabled = false;
                     preferences.putBool("bt_enabled", bluetoothEnabled);
                    
                    if (bluetoothInitialized) {
                        shutdownBluetooth();
                    }
                    
                    Serial.println("Bluetooth disabled and stopped immediately");
                    server.send(200, "text/plain", "Bluetooth disabled successfully");
                    
                } else {
                    // No change needed
                    String status = bluetoothEnabled ? "enabled" : "disabled";
                    server.send(200, "text/plain", "Bluetooth already " + status);
                }
                return;
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });

    // Ghost Key/Power system toggle endpoints
    server.on("/system_status", HTTP_GET, [](){
        Serial.println("System status request received");
        String json = "{";
        json += "\"ghostKeyEnabled\":" + String(ghostKeyEnabled ? "true" : "false") + ",";
        json += "\"ghostPowerEnabled\":" + String(ghostPowerEnabled ? "true" : "false");
        json += "}";
        server.send(200, "application/json", json);
    });
    
    server.on("/toggle_ghost_key", HTTP_POST, [](){
        Serial.println("Ghost Key toggle request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                bool newState = doc["enabled"];
                
                // Prevent disabling both systems
                if (!newState && !ghostPowerEnabled) {
                    server.send(400, "text/plain", "Cannot disable both Ghost Key and Ghost Power systems");
                    return;
                }
                
                ghostKeyEnabled = newState;
                preferences.putBool("ghost_key_enabled", ghostKeyEnabled);
                
                Serial.printf("Ghost Key %s\n", ghostKeyEnabled ? "enabled" : "disabled");
                String message = ghostKeyEnabled ? "Ghost Key enabled." : "Ghost Key disabled.";
                server.send(200, "text/plain", message);
                return;
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });
    
    server.on("/toggle_ghost_power", HTTP_POST, [](){
        Serial.println("Ghost Power toggle request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                bool newState = doc["enabled"];
                
                // Prevent disabling both systems
                if (!newState && !ghostKeyEnabled) {
                    server.send(400, "text/plain", "Cannot disable both Ghost Key and Ghost Power systems");
                    return;
                }
                
                ghostPowerEnabled = newState;
                preferences.putBool("ghost_power_enabled", ghostPowerEnabled);
                
                Serial.printf("Ghost Power %s\n", ghostPowerEnabled ? "enabled" : "disabled");
                String message = ghostPowerEnabled ? "Ghost Power enabled." : "Ghost Power disabled.";
                server.send(200, "text/plain", message);
                return;
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });

    // Route for exiting config mode
    server.on("/exit", HTTP_POST, [](){
        exitConfigMode();
        server.send(200, "text/plain", "Exiting config mode...");
    });

    // Setup completion endpoint - marks first setup as complete
    server.on("/complete_setup", HTTP_POST, [](){
        Serial.println("Setup completion request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                // Apply all the setup settings
                bool ghostKey = doc["ghostKeyEnabled"];
                bool ghostPower = doc["ghostPowerEnabled"];
                bool bluetooth = doc["bluetoothEnabled"];
                String wifiPass = doc["wifiPassword"];
                String webPass = doc["webPassword"];
                
                // Validate settings
                if ((!ghostKey && !ghostPower) || wifiPass.length() < 8 || webPass.length() < 4) {
                    server.send(400, "text/plain", "Invalid settings - check requirements");
                    return;
                }
                
                // Save all settings
                ghostKeyEnabled = ghostKey;
                ghostPowerEnabled = ghostPower;
                bluetoothEnabled = bluetooth;
                ap_password = wifiPass;
                web_password = webPass;
                
                preferences.putBool("ghost_key_enabled", ghostKeyEnabled);
                preferences.putBool("ghost_power_enabled", ghostPowerEnabled);
                preferences.putBool("bt_enabled", bluetoothEnabled);
                preferences.putString("wifi_password", ap_password);
                preferences.putString("web_password", web_password);
                
                // Mark setup as complete
                firstSetupComplete = true;
                preferences.putBool("first_setup_complete", true);
                
                Serial.println("Setup completed successfully");
                server.send(200, "text/plain", "Setup completed successfully");
                return;
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });

    // Legacy RSSI Calibration endpoints removed - using confidence-based authentication only

    // RSSI analysis endpoint for confidence-based authentication monitoring
    server.on("/rssi_analysis", HTTP_GET, [](){
        Serial.println("RSSI analysis status request received");
        String json = getRSSIAnalysisJson();
        server.send(200, "application/json", json);
    });

    // Calibration system endpoints
    server.on("/calibration_start", HTTP_POST, [](){
        Serial.println("Calibration start request received");
        if (!bluetoothEnabled || !bluetoothInitialized) {
            server.send(400, "text/plain", "Bluetooth is disabled");
            return;
        }
        if (isCalibrating) {
            server.send(400, "text/plain", "Calibration already in progress");
            return;
        }
        startCalibration();
        server.send(200, "text/plain", "Calibration started - position your phone and wait 30 seconds");
    });
    
    server.on("/calibration_stop", HTTP_POST, [](){
        Serial.println("Calibration stop request received");
        if (!isCalibrating) {
            server.send(400, "text/plain", "No calibration in progress");
            return;
        }
        stopCalibration();
        server.send(200, "text/plain", "Calibration stopped and offset calculated");
    });
    
    server.on("/calibration_reset", HTTP_POST, [](){
        Serial.println("Calibration reset request received");
        resetCalibration();
        server.send(200, "text/plain", "Calibration reset to default");
    });
    
    server.on("/calibration_status", HTTP_GET, [](){
        String json = "{";
        json += "\"isCalibrating\":" + String(isCalibrating ? "true" : "false") + ",";
        json += "\"offset\":" + String(calibrationOffset, 1) + ",";
        json += "\"sampleCount\":" + String(calibrationSampleCount) + ",";
        if (isCalibrating) {
            unsigned long elapsed = millis() - calibrationStartTime;
            unsigned long remaining = (elapsed < calibrationDuration) ? (calibrationDuration - elapsed) : 0;
            json += "\"timeRemaining\":" + String(remaining);
        } else {
            json += "\"timeRemaining\":0";
        }
        json += "}";
        server.send(200, "application/json", json);
    });

    // Start server
    server.begin();
    Serial.println("Web server started successfully");
    Serial.print("Server listening on IP: ");
    Serial.println(WiFi.softAPIP());
}

void setupBluetooth() {
    // Bluetooth initialization is now handled in initializeBluetooth()
    // This function kept for compatibility but does nothing
}

// updateAccessoryAuthentication - Check brake + accessory input for Ghost Power only mode
// Called from: main loop() when ghostKeyEnabled is false
// Links to: Used for Ghost Power only authentication
void updateAccessoryAuthentication() {
    bool brakeHigh = (digitalRead(BRAKE_PIN) == HIGH);
    bool accessoryHigh = (digitalRead(ACCESSORY_INPUT_PIN) == HIGH);
    
    // Authentication occurs when both brake and accessory inputs are HIGH
    bool newAuthState = (brakeHigh && accessoryHigh);
    
    if (newAuthState != accessoryInputAuth) {
        accessoryInputAuth = newAuthState;
        Serial.printf("Ghost Power authentication: %s (Brake: %s, Accessory: %s)\n", 
                     accessoryInputAuth ? "Authenticated" : "Not authenticated",
                     brakeHigh ? "HIGH" : "LOW",
                     accessoryHigh ? "HIGH" : "LOW");
    }
}

// updateBluetoothAuthentication - Check if connected device is close enough for auth
// Called from: main loop() continuously
// Links to: Uses RSSI thresholds, getDeviceRSSI(), sets bluetoothAuthenticated
void updateBluetoothAuthentication() {
    bool wasAuthenticated = bluetoothAuthenticated;
    
    // Need: connection + completed auth + valid address + close enough RSSI
    bluetoothAuthenticated = false;
    
    if (isBleConnected && hasConnectedDevice) {
        // Check for valid device address (cached for efficiency)
        static bool hasValidAddress = false;
        static esp_bd_addr_t lastCheckedAddr = {0};
        
        if (memcmp(connectedDeviceAddr, lastCheckedAddr, sizeof(esp_bd_addr_t)) != 0) {
            hasValidAddress = false;
            for (int i = 0; i < 6; i++) {
                if (connectedDeviceAddr[i] != 0) {
                    hasValidAddress = true;
                    break;
                }
            }
            memcpy(lastCheckedAddr, connectedDeviceAddr, sizeof(esp_bd_addr_t));
        }
        
        if (hasValidAddress) {
            // Use confidence-based authentication exclusively
            bluetoothAuthenticated = authenticateByConfidence();
        }
    }
    
    // Log changes
    if (wasAuthenticated != bluetoothAuthenticated) {
        Serial.printf("Bluetooth authentication: %s\n", bluetoothAuthenticated ? "Authenticated" : "Not authenticated");
        if (bluetoothAuthenticated) {
            int8_t rssi = 0;
            if (getDeviceRSSI(connectedDeviceAddr, &rssi)) {
                Serial.printf("Authenticated device: %02x:%02x:%02x:%02x:%02x:%02x (RSSI: %d dBm)\n",
                    connectedDeviceAddr[0], connectedDeviceAddr[1], connectedDeviceAddr[2],
                    connectedDeviceAddr[3], connectedDeviceAddr[4], connectedDeviceAddr[5], rssi);
            }
        }
    }
}

// updateSecurityState - Control security relays based on authentication
// Called from: main loop() every SECURITY_CHECK_INTERVAL
// Links to: Uses bluetoothAuthenticated, rfidAuthenticated, accessoryInputAuth, engineRunning
void updateSecurityState() {
    // Handle Ghost Key only mode - security relay always LOW (active)
    if (ghostKeyEnabled && !ghostPowerEnabled) {
        digitalWrite(RELAY_SECURITY, LOW); // Always active when Ghost Key only
        securityEnabled = true; // For status reporting
        return;
    }
    
    // Handle Ghost Power disabled (and Ghost Key also disabled - not allowed by validation)
    if (!ghostPowerEnabled) {
        digitalWrite(RELAY_SECURITY, HIGH); // Disabled when Ghost Power is off
        securityEnabled = false;
        return;
    }
    
    bool isAuthenticated = false;
    
    if (ghostKeyEnabled && ghostPowerEnabled) {
        // Both systems enabled: RFID/Bluetooth authentication only (no brake+accessory needed)
        isAuthenticated = rfidAuthenticated || (bluetoothEnabled && bluetoothAuthenticated);
    } else if (!ghostKeyEnabled && ghostPowerEnabled) {
        // Ghost Power only mode: Brake + Accessory authentication
        isAuthenticated = accessoryInputAuth;
    }
    
    // Security logic: enabled by default, disabled when authenticated or engine running
    if (engineRunning) {
        securityEnabled = false;
    } else if (isAuthenticated) {
        securityEnabled = false;
    } else {
        // No auth - enable security (with timeout after engine shutdown)
        if (lastEngineShutdown > 0) {
            unsigned long timeSinceShutdown = millis() - lastEngineShutdown;
            if (timeSinceShutdown >= autoLockTimeout) {
                securityEnabled = true;
            }
        } else {
            securityEnabled = true;
        }
    }
    
    // Control security relays
    if (securityEnabled) {
        digitalWrite(RELAY_SECURITY, LOW);
    } else {
        digitalWrite(RELAY_SECURITY, HIGH);
    }
}

void printSystemStatus() {
    Serial.println("\nSystem State: ");
    if (engineRunning) {
        Serial.println("RUNNING");
    } else if (startRelayActive) {
        Serial.println("STARTING");
    } else {
        switch (systemState) {
            case 0:
                Serial.println("OFF");
                break;
            case 1:
                Serial.println("ACCESSORY");
                break;
            case 2:
                Serial.println("IGNITION");
                break;
        }
    }
    
    Serial.print("\nLED States - ACC: ");
    Serial.print(digitalRead(RELAY_ACCESSORY) ? "ON" : "OFF");
    Serial.print(" IGN1: ");
    Serial.print(digitalRead(RELAY_IGNITION1) ? "ON" : "OFF");
    Serial.print(" IGN2: ");
    Serial.print(digitalRead(RELAY_IGNITION2) ? "ON" : "OFF");
    Serial.print(" START: ");
    Serial.println(digitalRead(RELAY_START) ? "ON" : "OFF");
    
    Serial.print("\nSecurity State: ");
    Serial.println(securityEnabled ? "ENABLED" : "DISABLED");
    Serial.print("Security Relays - POS: ");
    Serial.print(digitalRead(RELAY_SECURITY) ? "HIGH \n" : "LOW \n");
    
    Serial.print("\nBluetooth State: ");
    Serial.print(bluetoothEnabled ? "ENABLED" : "DISABLED");
    if (bluetoothEnabled) {
        Serial.print(" (");
        Serial.print(bluetoothInitialized ? "Initialized" : "Not Initialized");
        Serial.print(", Auth: ");
        Serial.print(bluetoothAuthenticated ? "YES" : "NO");
        Serial.println(")");
    } else {
        Serial.println();
    }
    
    Serial.print("\nSystem Configuration:");
    Serial.print("\n  Ghost Key: ");
    Serial.print(ghostKeyEnabled ? "ENABLED" : "DISABLED");
    Serial.print(" (RFID/Bluetooth/Push-to-start)");
    Serial.print("\n  Ghost Power: ");
    Serial.print(ghostPowerEnabled ? "ENABLED" : "DISABLED");
    Serial.print(" (Security relays)");
    
    if (!ghostKeyEnabled && ghostPowerEnabled) {
        Serial.print("\n  Accessory Input Auth: ");
        Serial.print(accessoryInputAuth ? "AUTHENTICATED" : "NOT AUTHENTICATED");
        Serial.print(" (Brake: ");
        Serial.print(digitalRead(BRAKE_PIN) == HIGH ? "HIGH" : "LOW");
        Serial.print(", Accessory: ");
        Serial.print(digitalRead(ACCESSORY_INPUT_PIN) == HIGH ? "HIGH" : "LOW");
        Serial.print(")");
    }
    Serial.println();
}

// checkFactoryReset - Detect start + brake held for 30 seconds to factory reset
// Called from: main loop() continuously
// Links to: Resets all settings except RFID keys
void checkFactoryReset() {
    bool startPressed = (digitalRead(BUTTON_PIN) == LOW);
    bool brakePressed = (digitalRead(BRAKE_PIN) == LOW);
    
    if (startPressed && brakePressed) {
        if (!factoryResetInProgress) {
            factoryResetInProgress = true;
            factoryResetStartTime = millis();
            DEBUG_PRINTLN("Factory reset sequence started - hold for 30 seconds");
        } else {
            unsigned long holdDuration = millis() - factoryResetStartTime;
            if (holdDuration >= FACTORY_RESET_TIME) {
                performFactoryReset();
            } else {
                // Visual feedback during reset sequence
                static unsigned long lastFeedback = 0;
                if (millis() - lastFeedback >= 1000) {
                    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Toggle LED
                    DEBUG_PRINT("Factory reset in progress: ");
                    DEBUG_PRINT((FACTORY_RESET_TIME - holdDuration) / 1000);
                    DEBUG_PRINTLN(" seconds remaining");
                    lastFeedback = millis();
                }
            }
        }
    } else {
        if (factoryResetInProgress) {
            factoryResetInProgress = false;
            digitalWrite(LED_PIN, LOW); // Turn off LED
            DEBUG_PRINTLN("Factory reset sequence cancelled");
        }
    }
}

// performFactoryReset - Reset all settings to defaults except RFID keys
// Called from: checkFactoryReset() when conditions are met
// Links to: Clears preferences, keeps RFID keys intact
void performFactoryReset() {
    DEBUG_PRINTLN("=== PERFORMING FACTORY RESET ===");
    
    // Visual feedback - rapid LED flashing
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
    
    // Reset all preferences except RFID keys
    preferences.clear(); // This clears everything
    
    // Restore RFID keys if they exist
    loadStoredRfidKeys(); // This will restore from cleared storage (empty)
    // Note: RFID keys are preserved since they're handled separately
    
    // Set defaults
    preferences.putBool("configured", false);
    preferences.putBool("bluetooth_paired", false);
    preferences.putBool("first_setup_complete", false);
    preferences.putULong("starter_pulse", STARTER_PULSE_TIME);
    preferences.putULong("auto_lock_timeout", AUTO_LOCK_TIMEOUT);
    preferences.putString("wifi_password", "123456789");
    preferences.putString("web_password", "1234");
    preferences.putBool("bt_enabled", true);
    preferences.putBool("ghost_key_enabled", true);
    preferences.putBool("ghost_power_enabled", true);
    
    // Clean up Bluetooth bonds
    cleanupNVSStorage();
    
    DEBUG_PRINTLN("Factory reset complete - restarting system");
    
    // Visual confirmation
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
    
    // Restart the system
    ESP.restart();
}

// ========================================
// LEGACY RSSI CALIBRATION FUNCTIONS REMOVED
// ========================================
// All RSSI calibration functions have been removed.
// System now uses confidence-based authentication exclusively.

// ========================================
// STATISTICAL RSSI ANALYSIS FUNCTIONS  
// ========================================

// Median filter for outlier rejection
int8_t medianFilter(int8_t newValue) {
    static int8_t medianBuffer[MEDIAN_FILTER_SIZE] = {-99, -99, -99, -99, -99};
    static int bufferIndex = 0;
    
    // Add new value to buffer
    medianBuffer[bufferIndex] = newValue;
    bufferIndex = (bufferIndex + 1) % MEDIAN_FILTER_SIZE;
    
    // Sort buffer for median calculation
    int8_t sortedBuffer[MEDIAN_FILTER_SIZE];
    memcpy(sortedBuffer, medianBuffer, sizeof(medianBuffer));
    
    // Simple bubble sort for small array
    for (int i = 0; i < MEDIAN_FILTER_SIZE - 1; i++) {
        for (int j = 0; j < MEDIAN_FILTER_SIZE - i - 1; j++) {
            if (sortedBuffer[j] > sortedBuffer[j + 1]) {
                int8_t temp = sortedBuffer[j];
                sortedBuffer[j] = sortedBuffer[j + 1];
                sortedBuffer[j + 1] = temp;
            }
        }
    }
    
    // Return median value
    return sortedBuffer[MEDIAN_FILTER_SIZE / 2];
}

// Outlier rejection based on statistical analysis
bool isOutlier(int8_t rssi, const RSSIStatistics& stats) {
    if (stats.validSamples < MIN_READINGS_FOR_ANALYSIS) {
        return false; // Not enough data to determine outliers
    }
    
    float deviation = abs(rssi - stats.mean);
    float threshold = OUTLIER_REJECTION_STDDEV_MULT * stats.standardDeviation;
    
    return deviation > threshold;
}

// Calculate sample quality weight based on number of valid samples
float calculateSampleQualityWeight(int validSamples) {
    if (validSamples <= 0) {
        return 0.0f;
    }
    
    if (validSamples >= OPTIMAL_SAMPLE_COUNT) {
        return 1.0f;
    }
    
    // Linear interpolation between minimum weight and full weight
    float ratio = (float)validSamples / OPTIMAL_SAMPLE_COUNT;
    return MIN_SAMPLE_QUALITY_WEIGHT + (1.0f - MIN_SAMPLE_QUALITY_WEIGHT) * ratio;
}

// Sigmoid function for smooth signal strength calculation
float sigmoidSignalStrength(float rssi) {
    // Sigmoid function: f(x) = 1 / (1 + e^(-k(x-x0)))
    // Where k = steepness, x0 = midpoint
    float exponent = -SIGMOID_STEEPNESS * (rssi - SIGMOID_MIDPOINT);
    return 1.0f / (1.0f + exp(exponent));
}

// Add RSSI reading to analysis buffers
void addRSSIReading(int8_t rssi, esp_bd_addr_t address) {
    if (!isRSSIValid(rssi)) {
        return;
    }
    
    unsigned long now = millis();
    
    // Set up device tracking if not already set
    if (!rssiAnalysis.hasDevice) {
        memcpy(rssiAnalysis.deviceAddress, address, sizeof(esp_bd_addr_t));
        rssiAnalysis.hasDevice = true;
    } else if (memcmp(rssiAnalysis.deviceAddress, address, sizeof(esp_bd_addr_t)) != 0) {
        // Different device - reset analysis
        resetRSSIAnalysis();
        memcpy(rssiAnalysis.deviceAddress, address, sizeof(esp_bd_addr_t));
        rssiAnalysis.hasDevice = true;
    }
    
    // Apply median filter for outlier rejection
    int8_t filteredRSSI = medianFilter(rssi);
    
    // Check for outliers against current statistics (for logging purposes)
    bool isRSSIOutlier = isOutlier(filteredRSSI, rssiAnalysis.shortTermStats);
    if (isRSSIOutlier && rssiAnalysis.shortTermStats.validSamples >= MIN_READINGS_FOR_ANALYSIS) {
        static unsigned long lastOutlierLog = 0;
        if (now - lastOutlierLog > 5000) { // Log outliers every 5 seconds max
            Serial.printf("BLE: Outlier detected - Raw: %d dBm, Filtered: %d dBm\n", rssi, filteredRSSI);
            lastOutlierLog = now;
        }
    }
    
    // Update signal tracking
    rssiAnalysis.lastSignalTime = now;
    rssiAnalysis.signalLost = false;
    
    // Create new reading with filtered value
    RSSIReading reading;
    reading.rssi = filteredRSSI;
    reading.timestamp = now;
    reading.valid = true;
    
    // Add to short term buffer (1 second)
    rssiAnalysis.shortTerm[rssiAnalysis.shortTermIndex] = reading;
    rssiAnalysis.shortTermIndex = (rssiAnalysis.shortTermIndex + 1) % RSSI_SHORT_TERM_SIZE;
    
    // Add to medium term buffer (5 seconds) 
    rssiAnalysis.mediumTerm[rssiAnalysis.mediumTermIndex] = reading;
    rssiAnalysis.mediumTermIndex = (rssiAnalysis.mediumTermIndex + 1) % RSSI_MEDIUM_TERM_SIZE;
    
    // Add to long term buffer (30 seconds)
    rssiAnalysis.longTerm[rssiAnalysis.longTermIndex] = reading;
    rssiAnalysis.longTermIndex = (rssiAnalysis.longTermIndex + 1) % RSSI_LONG_TERM_SIZE;
    
    rssiAnalysis.lastUpdate = now;
    
    // Trigger analysis if enough time has passed
    if (now - rssiAnalysis.lastAnalysis >= 250) { // Analyze every 250ms (4x faster response)
        performRSSIAnalysis();
        rssiAnalysis.lastAnalysis = now;
    }
}

// Calculate statistics for a buffer
RSSIStatistics calculateBufferStatistics(RSSIReading* buffer, int bufferSize, unsigned long maxAge) {
    RSSIStatistics stats;
    unsigned long now = millis();
    
    float sum = 0.0f;
    int validCount = 0;
    stats.minimum = 0;
    stats.maximum = -100;
    
    // First pass: calculate mean and find min/max
    for (int i = 0; i < bufferSize; i++) {
        if (buffer[i].valid && (now - buffer[i].timestamp) <= maxAge) {
            float rssi = (float)buffer[i].rssi;
            sum += rssi;
            validCount++;
            
            if (validCount == 1 || buffer[i].rssi > stats.maximum) {
                stats.maximum = buffer[i].rssi;
            }
            if (validCount == 1 || buffer[i].rssi < stats.minimum) {
                stats.minimum = buffer[i].rssi;
            }
        }
    }
    
    stats.validSamples = validCount;
    
    if (validCount < MIN_READINGS_FOR_ANALYSIS) {
        return stats; // Not enough data
    }
    
    stats.mean = sum / validCount;
    
    // Second pass: calculate variance and standard deviation
    float varianceSum = 0.0f;
    for (int i = 0; i < bufferSize; i++) {
        if (buffer[i].valid && (now - buffer[i].timestamp) <= maxAge) {
            float diff = (float)buffer[i].rssi - stats.mean;
            varianceSum += diff * diff;
        }
    }
    
    stats.variance = varianceSum / validCount;
    stats.standardDeviation = sqrt(stats.variance);
    
    return stats;
}

// Get adaptive stability threshold based on signal strength
float getAdaptiveStabilityThreshold(float meanRSSI) {
    if (meanRSSI >= -50) return 15.0f;      // Very close: allow more variance
    if (meanRSSI >= -65) return 12.0f;      // Close: current tolerance
    if (meanRSSI >= -75) return 8.0f;       // Medium: less tolerance
    return 5.0f;                            // Far: strict tolerance
}

// Detect stationary phone with strong signal
bool isStationaryStrongSignal(const RSSIStatistics& stats) {
    return (stats.mean >= -55 && 
            stats.standardDeviation <= STATIONARY_MAX_STDDEV && 
            stats.validSamples >= STATIONARY_MIN_SAMPLES);
}

// Calculate stability score (0-35 points) with distance-dependent scaling
float calculateStabilityScore() {
    // Use short-term statistics for stability
    if (rssiAnalysis.shortTermStats.validSamples < MIN_READINGS_FOR_ANALYSIS) {
        return 0.0f;
    }
    
    float meanRSSI = rssiAnalysis.shortTermStats.mean;
    float stdDev = rssiAnalysis.shortTermStats.standardDeviation;
    
    // Get adaptive threshold based on signal strength (keep existing logic for stability detection)
    float adaptiveThreshold = getAdaptiveStabilityThreshold(meanRSSI);
    
    // Calculate base stability score using existing logic
    float baseScore = 0.0f;
    
    if (stdDev <= adaptiveThreshold) {
        // Signal is stable - full points before distance scaling
        baseScore = STABILITY_WEIGHT;
    } else {
        // Signal is unstable - reduce points proportionally
        float stabilityRatio = adaptiveThreshold / stdDev;
        baseScore = STABILITY_WEIGHT * stabilityRatio;
    }
    
    // Ensure base score doesn't go negative
    if (baseScore < 0.0f) {
        baseScore = 0.0f;
    }
    
    // Apply distance-dependent scaling based on RSSI (eased thresholds)
    float distanceMultiplier = 1.0f;
    
    if (meanRSSI >= -60) {
        // Very Close: Full stability points
        distanceMultiplier = 1.0f;
    } else if (meanRSSI >= -70) {
        // Close: 85% of stability points (was 70%)
        distanceMultiplier = 0.85f;
    } else if (meanRSSI >= -80) {
        // Medium: 60% of stability points (was 40%)
        distanceMultiplier = 0.6f;
    } else if (meanRSSI >= -90) {
        // Far: 25% of stability points (was 15%)
        distanceMultiplier = 0.25f;
    } else {
        // Very Far: 5% of stability points (was 0%)
        distanceMultiplier = 0.05f;
    }
    
    // Apply sample quality weighting
    float qualityWeight = calculateSampleQualityWeight(rssiAnalysis.shortTermStats.validSamples);
    
    // Small base bonus for any level of stability (2 points), regardless of distance
    float finalScore = (baseScore * distanceMultiplier * qualityWeight) + (rssiAnalysis.isStable ? 2.0f : 0.0f);
    
    // Ensure final score doesn't exceed maximum
    if (finalScore > STABILITY_WEIGHT) {
        finalScore = STABILITY_WEIGHT;
    }
    
    return finalScore;
}

// Calculate trend score (0-25 points) - Updated to be friendlier to stationary phones
float calculateTrendScore() {
    // Need medium-term data for trend analysis
    if (rssiAnalysis.mediumTermStats.validSamples < MIN_READINGS_FOR_ANALYSIS) {
        return 0.0f;
    }
    
    // Calculate trend direction using linear regression over recent readings
    unsigned long now = millis();
    float sumX = 0.0f, sumY = 0.0f, sumXY = 0.0f, sumXX = 0.0f;
    int trendSamples = 0;
    
    for (int i = 0; i < RSSI_MEDIUM_TERM_SIZE; i++) {
        if (rssiAnalysis.mediumTerm[i].valid && 
            (now - rssiAnalysis.mediumTerm[i].timestamp) <= TREND_WINDOW_MS) {
            
            float x = (float)(now - rssiAnalysis.mediumTerm[i].timestamp); // Time ago
            float y = (float)rssiAnalysis.mediumTerm[i].rssi;
            
            sumX += x;
            sumY += y;
            sumXY += x * y;
            sumXX += x * x;
            trendSamples++;
        }
    }
    
    if (trendSamples < MIN_READINGS_FOR_ANALYSIS) {
        return 0.0f;
    }
    
    // Linear regression slope calculation
    float slope = (trendSamples * sumXY - sumX * sumY) / (trendSamples * sumXX - sumX * sumX);
    
    // Add slope to history for averaging (hysteresis filter)
    rssiAnalysis.slopeHistory[rssiAnalysis.slopeHistoryIndex] = slope;
    rssiAnalysis.slopeHistoryIndex = (rssiAnalysis.slopeHistoryIndex + 1) % SLOPE_HISTORY_SIZE;
    
    // Calculate averaged slope for stability
    float slopeSum = 0.0f;
    for (int i = 0; i < SLOPE_HISTORY_SIZE; i++) {
        slopeSum += rssiAnalysis.slopeHistory[i];
    }
    rssiAnalysis.averagedSlope = slopeSum / SLOPE_HISTORY_SIZE;
    rssiAnalysis.trendDirection = rssiAnalysis.averagedSlope;
    
    // Use averaged slope for direction detection (prevents noise reactions)
    rssiAnalysis.isApproaching = (rssiAnalysis.averagedSlope > 0.005f);
    
    // Score based on trend strength and direction
    float trendStrength = abs(slope) * 1000.0f; // Scale for scoring
    float score = 0.0f;
    
    // Check if signal is very strong (close proximity)
    float meanRSSI = rssiAnalysis.mediumTermStats.mean;
    bool isVeryClose = (meanRSSI >= VERY_STRONG_SIGNAL_THRESHOLD);
    
    if (rssiAnalysis.isApproaching) {
        // Reward positive trends (approaching)
        score = TREND_WEIGHT * min(1.0f, trendStrength / 5.0f);
    } else if (isVeryClose && trendStrength < 2.0f) {
        // Give high points for stable signal when very close (stationary phone bonus)
        score = TREND_WEIGHT * 0.8f; // 80% of trend points for stable close proximity
    } else {
        // Penalize negative trends (leaving), but give some points for stability
        score = TREND_WEIGHT * 0.4f * max(0.0f, 1.0f - trendStrength / 5.0f);
    }
    
    return score;
}

// Calculate signal strength score (0-40 points) - Enhanced with sigmoid function
float calculateStrengthScore() {
    if (rssiAnalysis.shortTermStats.validSamples < MIN_READINGS_FOR_ANALYSIS) {
        return 0.0f;
    }
    
    float meanRSSI = rssiAnalysis.shortTermStats.mean;
    
    // Use sigmoid function for smooth scoring
    float sigmoidValue = sigmoidSignalStrength(meanRSSI);
    
    // Apply sample quality weighting
    float qualityWeight = calculateSampleQualityWeight(rssiAnalysis.shortTermStats.validSamples);
    rssiAnalysis.sampleQualityWeight = qualityWeight;
    
    // Calculate base score using sigmoid (smoother than step function)
    float baseScore = STRENGTH_WEIGHT * sigmoidValue;
    
    // Apply quality weighting
    float finalScore = baseScore * qualityWeight;
    
    return finalScore;
}

// Perform complete RSSI analysis and confidence calculation
void performRSSIAnalysis() {
    unsigned long now = millis();
    
    // Check for signal loss and handle no-signal decay
    if (now - rssiAnalysis.lastSignalTime > NO_SIGNAL_TIMEOUT_MS) {
        if (!rssiAnalysis.signalLost) {
            rssiAnalysis.signalLost = true;
            Serial.println("BLE: Signal lost - applying decay");
        }
        
        // Apply no-signal decay to confidence
        float decayRate = (now - rssiAnalysis.lastSignalTime > NO_SIGNAL_TIMEOUT_MS * 2) ? 
                         SIGNAL_LOSS_FAST_DECAY_RATE : NO_SIGNAL_DECAY_RATE;
        
        rssiAnalysis.totalConfidence *= decayRate;
        
        // Ensure confidence doesn't go below 0
        if (rssiAnalysis.totalConfidence < 0.0f) {
            rssiAnalysis.totalConfidence = 0.0f;
        }
        
        // Skip normal analysis when signal is lost
        return;
    }
    
    // Calculate statistics for each time window
    rssiAnalysis.shortTermStats = calculateBufferStatistics(
        rssiAnalysis.shortTerm, RSSI_SHORT_TERM_SIZE, 2000); // 2 second max age
    
    rssiAnalysis.mediumTermStats = calculateBufferStatistics(
        rssiAnalysis.mediumTerm, RSSI_MEDIUM_TERM_SIZE, 10000); // 10 second max age
    
    rssiAnalysis.longTermStats = calculateBufferStatistics(
        rssiAnalysis.longTerm, RSSI_LONG_TERM_SIZE, 60000); // 60 second max age
    
    // Calculate confidence components
    rssiAnalysis.stabilityScore = calculateStabilityScore();
    rssiAnalysis.trendScore = calculateTrendScore();
    rssiAnalysis.strengthScore = calculateStrengthScore();
    
    // Calculate base confidence
    float baseConfidence = rssiAnalysis.stabilityScore + 
                          rssiAnalysis.trendScore + 
                          rssiAnalysis.strengthScore;
    
    // Apply stationary strong signal bonus
    if (isStationaryStrongSignal(rssiAnalysis.shortTermStats)) {
        baseConfidence += STATIONARY_BONUS_POINTS;
    }
    
    // Apply calibration offset to base confidence (bias-corrected smoothing)
    float adjustedBaseConfidence = baseConfidence + calibrationOffset;
    
    // Store confidence before smoothing for calibration sampling
    float confidenceBeforeSmoothing = adjustedBaseConfidence;
    
    // Calculate adaptive momentum rate based on context
    float confidenceDelta = adjustedBaseConfidence - lastConfidence;
    float adaptiveMomentumRate;
    
    // Check if user is clearly moving away (weak signal + dropping confidence)
    bool clearlyLeaving = (rssiAnalysis.shortTermStats.mean < -80 && confidenceDelta < -5.0f);
    
    // Check if user is stationary with good signal (stable for smoothness)
    bool stationaryGoodSignal = (rssiAnalysis.isStable && rssiAnalysis.shortTermStats.mean > -75);
    
    if (clearlyLeaving) {
        // Fast deauth when clearly moving away
        adaptiveMomentumRate = CONFIDENCE_MOMENTUM_RATE_FAST;
    } else if (stationaryGoodSignal) {
        // Extra smooth when stationary with good signal
        adaptiveMomentumRate = 0.12f; // Slower than normal for extra smoothness
    } else if (abs(confidenceDelta) > CONFIDENCE_CHANGE_THRESHOLD) {
        // Fast response for big changes
        adaptiveMomentumRate = CONFIDENCE_MOMENTUM_RATE_FAST;
    } else {
        // Normal response
        adaptiveMomentumRate = CONFIDENCE_MOMENTUM_RATE;
    }
    
    // Apply adaptive momentum smoothing
    rssiAnalysis.totalConfidence = lastConfidence + (confidenceDelta * adaptiveMomentumRate);
    
    // Extra smoothing for authenticated users sitting still (reduce peakiness)
    if (bluetoothAuthenticated && stationaryGoodSignal && abs(confidenceDelta) < 5.0f) {
        // Apply additional smoothing to reduce small fluctuations
        float extraSmoothing = 0.8f; // Keep 80% of previous + 20% of new
        rssiAnalysis.totalConfidence = (lastConfidence * extraSmoothing) + 
                                     (rssiAnalysis.totalConfidence * (1.0f - extraSmoothing));
    }
    
    lastConfidence = rssiAnalysis.totalConfidence;
    
    // Ensure confidence stays within bounds
    if (rssiAnalysis.totalConfidence < 0.0f) rssiAnalysis.totalConfidence = 0.0f;
    if (rssiAnalysis.totalConfidence > 100.0f) rssiAnalysis.totalConfidence = 100.0f;
    
    // Collect calibration sample if in calibration mode
    if (isCalibrating) {
        addCalibrationSample(rssiAnalysis.shortTermStats.mean, confidenceBeforeSmoothing);
    }
    
    // Update stability flag using adaptive threshold
    float adaptiveThreshold = getAdaptiveStabilityThreshold(rssiAnalysis.shortTermStats.mean);
    rssiAnalysis.isStable = (rssiAnalysis.shortTermStats.standardDeviation <= adaptiveThreshold);
    
    // Debug logging (reduced frequency)
    static unsigned long lastDebugLog = 0;
    if (now - lastDebugLog >= 2000) { // Log every 2 seconds for faster feedback
        // Calculate distance multiplier for logging (same logic as in calculateStabilityScore)
        float distanceMultiplier = 1.0f;
        float meanRSSI = rssiAnalysis.shortTermStats.mean;
        if (meanRSSI >= -60) distanceMultiplier = 1.0f;
        else if (meanRSSI >= -70) distanceMultiplier = 0.85f;
        else if (meanRSSI >= -80) distanceMultiplier = 0.6f;
        else if (meanRSSI >= -90) distanceMultiplier = 0.25f;
        else distanceMultiplier = 0.05f;
        
        // Determine current adaptive mode for logging
        const char* adaptiveMode = "Normal";
        bool clearlyLeaving = (rssiAnalysis.shortTermStats.mean < -80 && (rssiAnalysis.totalConfidence - lastConfidence) < -5.0f);
        bool stationaryGoodSignal = (rssiAnalysis.isStable && rssiAnalysis.shortTermStats.mean > -75);
        
        if (clearlyLeaving) adaptiveMode = "FastDeauth";
        else if (stationaryGoodSignal) adaptiveMode = "ExtraSmooth";
        else if (abs(rssiAnalysis.totalConfidence - lastConfidence) > CONFIDENCE_CHANGE_THRESHOLD) adaptiveMode = "FastResponse";
        
        Serial.printf("RSSI Analysis: Confidence=%.1f%% (Stability=%.1f[x%.1f], Trend=%.1f, Strength=%.1f) "
                     "Mean=%.1f, StdDev=%.1f, Quality=%.1f%%, Mode=%s, Trending=%s%s\n",
                     rssiAnalysis.totalConfidence,
                     rssiAnalysis.stabilityScore, distanceMultiplier,
                     rssiAnalysis.trendScore, 
                     rssiAnalysis.strengthScore,
                     rssiAnalysis.shortTermStats.mean,
                     rssiAnalysis.shortTermStats.standardDeviation,
                     rssiAnalysis.sampleQualityWeight * 100.0f,
                     adaptiveMode,
                     rssiAnalysis.isApproaching ? "Approaching" : "Stable/Leaving",
                     rssiAnalysis.signalLost ? " [SIGNAL LOST]" : "");
        lastDebugLog = now;
    }
}

// Check if device should be authenticated based on confidence
bool authenticateByConfidence() {
    // Need minimum data for analysis
    if (rssiAnalysis.shortTermStats.validSamples < MIN_READINGS_FOR_ANALYSIS) {
        return false;
    }
    
    // Use hysteresis for authentication decisions
    static bool wasAuthenticated = false;
    float threshold = wasAuthenticated ? CONFIDENCE_DEAUTH_THRESHOLD : CONFIDENCE_AUTH_THRESHOLD;
    
    bool shouldAuthenticate = (rssiAnalysis.totalConfidence >= threshold);
    
    // Update state
    wasAuthenticated = shouldAuthenticate;
    
    return shouldAuthenticate;
}

// Reset RSSI analysis (when device changes or system resets)
void resetRSSIAnalysis() {
    memset(&rssiAnalysis, 0, sizeof(rssiAnalysis));
    
    // Reset statistics
    for (int i = 0; i < RSSI_SHORT_TERM_SIZE; i++) {
        rssiAnalysis.shortTerm[i].valid = false;
    }
    for (int i = 0; i < RSSI_MEDIUM_TERM_SIZE; i++) {
        rssiAnalysis.mediumTerm[i].valid = false;
    }
    for (int i = 0; i < RSSI_LONG_TERM_SIZE; i++) {
        rssiAnalysis.longTerm[i].valid = false;
    }
    
    // Initialize enhanced fields
    rssiAnalysis.sampleQualityWeight = 1.0f;
    rssiAnalysis.signalLost = false;
    rssiAnalysis.lastSignalTime = millis();
    for (int i = 0; i < SLOPE_HISTORY_SIZE; i++) {
        rssiAnalysis.slopeHistory[i] = 0.0f;
    }
    rssiAnalysis.slopeHistoryIndex = 0;
    rssiAnalysis.averagedSlope = 0.0f;
    
    // Reset global confidence tracking
    lastConfidence = 0.0f;
    
    Serial.println("RSSI Analysis: Enhanced reset for new device or system restart");
}

// Get RSSI analysis status as JSON for web interface
String getRSSIAnalysisJson() {
    String json = "{";
    json += "\"confidence\":" + String(rssiAnalysis.totalConfidence, 1) + ",";
    json += "\"stabilityScore\":" + String(rssiAnalysis.stabilityScore, 1) + ",";
    json += "\"trendScore\":" + String(rssiAnalysis.trendScore, 1) + ",";
    json += "\"strengthScore\":" + String(rssiAnalysis.strengthScore, 1) + ",";
    json += "\"isApproaching\":" + String(rssiAnalysis.isApproaching ? "true" : "false") + ",";
    json += "\"isStable\":" + String(rssiAnalysis.isStable ? "true" : "false") + ",";
    json += "\"mean\":" + String(rssiAnalysis.shortTermStats.mean, 1) + ",";
    json += "\"stdDev\":" + String(rssiAnalysis.shortTermStats.standardDeviation, 1) + ",";
    json += "\"samples\":" + String(rssiAnalysis.shortTermStats.validSamples) + ",";
    json += "\"sampleQuality\":" + String(rssiAnalysis.sampleQualityWeight * 100.0f, 1) + ",";
    json += "\"signalLost\":" + String(rssiAnalysis.signalLost ? "true" : "false") + ",";
    json += "\"averagedSlope\":" + String(rssiAnalysis.averagedSlope, 4) + ",";
    json += "\"authThreshold\":" + String(CONFIDENCE_AUTH_THRESHOLD, 1) + ",";
    json += "\"deauthThreshold\":" + String(CONFIDENCE_DEAUTH_THRESHOLD, 1) + ",";
    json += "\"calibrationOffset\":" + String(calibrationOffset, 1) + ",";
    json += "\"isCalibrating\":" + String(isCalibrating ? "true" : "false");
    json += "}";
    return json;
}

// ========================================
// CALIBRATION SYSTEM FUNCTIONS
// ========================================

// Start calibration mode
void startCalibration() {
    if (isCalibrating) return;
    
    isCalibrating = true;
    calibrationStartTime = millis();
    calibrationSampleCount = 0;
    
    Serial.println("=== STARTING CONFIDENCE CALIBRATION ===");
    Serial.println("Position your phone where you want authentication to work");
    Serial.println("Collecting data for 30 seconds...");
}

// Stop calibration mode and calculate offset
void stopCalibration() {
    if (!isCalibrating) return;
    
    isCalibrating = false;
    
    if (calibrationSampleCount < 10) {
        Serial.println("Calibration failed: Not enough samples collected");
        return;
    }
    
    // Calculate average confidence during calibration period
    float avgConfidence = 0.0f;
    float avgRSSI = 0.0f;
    
    for (int i = 0; i < calibrationSampleCount; i++) {
        avgConfidence += calibrationConfidenceReadings[i];
        avgRSSI += calibrationRSSIReadings[i];
    }
    avgConfidence /= calibrationSampleCount;
    avgRSSI /= calibrationSampleCount;
    
    // Calculate what offset is needed to reach 72% confidence (safely above 65% threshold)
    float targetConfidence = 72.0f;
    float newOffset = targetConfidence - avgConfidence;
    
    // Limit offset to reasonable range (-30 to +30)
    if (newOffset > 30.0f) newOffset = 30.0f;
    if (newOffset < -30.0f) newOffset = -30.0f;
    
    calibrationOffset = newOffset;
    
    // Save to preferences
    preferences.putFloat("calibration_offset", calibrationOffset);
    
    Serial.println("=== CALIBRATION COMPLETE ===");
    Serial.printf("Samples collected: %d\n", calibrationSampleCount);
    Serial.printf("Average RSSI: %.1f dBm\n", avgRSSI);
    Serial.printf("Average confidence (before offset): %.1f%%\n", avgConfidence);
    Serial.printf("Calculated offset: %.1f\n", calibrationOffset);
    Serial.printf("Target confidence (with offset): %.1f%%\n", avgConfidence + calibrationOffset);
    Serial.println("===============================");
}

// Reset calibration to default
void resetCalibration() {
    calibrationOffset = 0.0f;
    preferences.putFloat("calibration_offset", calibrationOffset);
    Serial.println("Calibration reset to default (no offset)");
}

// Add calibration sample during calibration mode
void addCalibrationSample(float rssi, float confidence) {
    if (!isCalibrating || calibrationSampleCount >= MAX_CALIBRATION_SAMPLES) {
        return;
    }
    
    calibrationRSSIReadings[calibrationSampleCount] = rssi;
    calibrationConfidenceReadings[calibrationSampleCount] = confidence;
    calibrationSampleCount++;
    
    // Auto-stop after duration
    if (millis() - calibrationStartTime >= calibrationDuration) {
        stopCalibration();
    }
}

// ========================================
// BLE SERVER CALLBACK CLASS
// ========================================