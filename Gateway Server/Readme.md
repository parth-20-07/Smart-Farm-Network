## Accessing DigitalOcean via Putty

1. Download Putty from <https://www.putty.org/>.
2. Fill the IP address as ***178.62.225.6*** and leave everything as default in Putty Wind.
3. After opening and connecting to that IP address via Putty, you have to login as:
   - User ID: **root**
   - Password: **jAikishan@7P**
4. After successful login on the Putty terminal, in order to see the published data, type:

     ```$ mosquitto_sub -h localhost -t test -u "Citygreens_Gateway" -P "citygreens@123"```