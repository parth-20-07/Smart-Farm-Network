#ifndef HTML_PAGE_H
#define HTML_PAGE_H
// HTML web page to handle input fields
const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>

<head>
    <title>Gateway Configuration Page</title>
    <meta name="viewport" http-equiv="Content-Type"
        content="width=device-width, initial-scale=1 text/html; charset=utf-8">
    <style>
        body {
            background-color: #0000008c;
            font-family: Georgia, "Times New Roman", Times, serif;
            color: rgb(255, 255, 255);
        }

        input[type="text"] {
            width: 100px;
            height: 20px;
            border: 1px solid rgba(0, 0, 0, 0.349);
            background: #ffffff;
        }

        input[type="number"] {
            width: 100px;
            height: 20px;
            border: 1px solid rgba(0, 0, 0, 0.349);
            background: #ffffff;
        }
    </style>
</head>

<body>
    <center>
        <form action="/get">
            <h2 align="center">Gateway Configuration Form</h2>

            <h3>Enter Wi-Fi Details

            </h3>
            <table>
                <tr>
                    <td align="right">SSID:</td>
                    <td><input type="text" name="ssid" placeholder="Enter SSID" /></td>
                </tr>

                <tr>
                    <td align="right">Password:</td>
                    <td>
                        <input type="text" name="password" placeholder="Enter Password" />
                    </td>
                </tr>
            </table>

            <h3>Enter Sensor Range</h3>
            <table>
                <tr>
                    <td align="right">Temperature (°C):</td>
                    <td>
                        <input type="number" name="min_temp" placeholder="Minimum" min="0" max="60" />
                    </td>
                    <td>
                        <input type="number" name="max_temp" placeholder="Maximum" min="0" max="60" />
                    </td>
                </tr>

                <tr>
                    <td align="right">Humidity (%):</td>
                    <td>
                        <input type="number" name="min_humi" placeholder="Minimum" min="0" max="100" />
                    </td>
                    <td>
                        <input type="number" name="max_humi" placeholder="Maximum" min="0" max="100" />
                    </td>
                </tr>

                <tr>
                    <td align="right">Luminosity (lux):</td>
                    <td>
                        <input type="number" name="min_lux" placeholder="Minimum" min="1" max="40000" />
                    </td>
                    <td>
                        <input type="number" name="max_lux" placeholder="Maximum" min="1" max="40000" />
                    </td>
                </tr>

                <tr>
                    <td align="right">Carbondioxide (ppm):</td>
                    <td>
                        <input type="number" name="min_co2" placeholder="Minimum" min="400" max="5000" />
                    </td>
                    <td>
                        <input type="number" name="max_co2" placeholder="Maximum" min="400" max="5000" />
                    </td>
                </tr>

            </table>
            <br />
            <input type="submit" value="Submit" />
        </form>
    </center>
</body>

</html>

)rawliteral";
#endif