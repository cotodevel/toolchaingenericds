Coto:

This covers the UDP NIFI protocol, to connect DS - DS through internet.

This is a (windows only) UDP Server called Server Companion, 

if you have VS2012+ installed you can edit sources through .sln file.



Steps:

1) use some router/old phone that supports either OPEN or WEP connections, setup a WIFI hotspot in that device. 
Use any DS game that supports NIFI to setup the DS to connect to the device you use as WIFI hotspot.

2) The computer that acts as server must use the IP: 192.168.43.220, that IP is hardcoded in the ToolchainGenericDS library (Nintendo DS -> dswnifi_lib.cpp [server_ip]), 
if you want you can set the IP your server uses (static I would suggest) in there, recompile TGDS + project so changes are made.
The Server Companion detects the IP used to Network in the current PC, so if you connect your PC to WIFI hotspot, that IP is the one you must reflect back in the file said above.

3) On the TGDS project you use this function to enter UDP NIFI mode:

//udp nifi:
switch_dswnifi_mode(dswifi_udpnifimode);

Once recompiled, you copy TGDS project to your DS, and run. But do not call the above call yet.

3) head to /bin folder, run ConsoleApplication1.exe, a console with yellow letters will pop-up saying to connect two (yes 2) DSs in the same network.

Now, while running the TGDS project, call the code shown in //udp nifi:
By then, the Windows console app running in the server companion should detect 1 DS connected. Repeat the same steps for the second DS. 
When two DSes are connected, they will connect each other!

Check the project specific folder: common/dswnifi.cpp file explaining how to override userCode sender, userCode ReceiveHandler so you add them to your project.

As of now the DSWNIFI library support local nifi and udp nifi.



Coto.