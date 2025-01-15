#include <Arduino.h>
const char html_change[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
	<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
	<title>ESP32 Configuration</title>
	<style>
		html{
			font-family: Helvetica;
			display: inline-block;
			margin: 0px auto;
			text-align: center;}
		body {margin-top: 50px;}
		p {
			font-size: 14px;
			color: #888;
			margin-bottom: 10px;}
		h2 {color: #444444;margin: 40px auto 30px;}
		.form-text {
			font-size: 14px;
			color: #444444;}
		.button {
			background-color: #008B8B;
			border: none;
			color: white;
			padding: 15px 30px;
			text-decoration: none;
			font-size: 15px;
			margin: 0px auto 35px;
			cursor: pointer;
			border-radius: 10px;}
		.button:hover {
			background: #004d4d;}
		input[type=file]::file-selector-button {
			margin-right: 20px;
			border: none;
			background: #008B8B;
			padding: 10px 20px;
			border-radius: 10px;
			color: #fff;
			cursor: pointer;
			transition: background .2s ease-in-out;}
		input[type=file]::file-selector-button:hover {
			background: #004d4d;}
		.prg-div {
			font-size: 14px;
			color: #444444;}
	</style>
</head>
<body>
	<h2>Wi-Fi credentials changed!</h2>
	<a href="/" class="button">Back</a>
</body>
</html>
)rawliteral";