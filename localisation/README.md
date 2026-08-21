<h1>Bienvenue sur le test de localisation depuis win32<br>pour récupérer la localisation d'une machine sur windows (v 10 minimum) avec winRT implémenté dans du code C brut.</h1>
<br>
<h2>localisationwin32v1.c et localisationwin32v1.exe est le premier essai fonctionnel (avec débogage).</h2>
<h2>localisationwin32v2.c et localisationwin32v2.dll est le code réduit en .dll pour une application win32 autre à implémentation facile.</h2>
<br>
<h2>Fonction de la dll</h2>
<ul>
<li>int geo_process(void)</li>
<li>HRESULT geo_get_hr(void)</li>
<li>const char *geo_get_error(void)</li>
<li>double geo_get_latitude(void)</li>
<li>double geo_get_longitude(void)</li>
<li>double geo_get_accuracy(void)</li>
<li>int geo_view(void)</li>
</ul>
<br> 

> [!TIP]  <h2>localisationwin32v2.dll peut être utiliser avec fr-simplecodeX.X.X.<br>Exemple : geolocation.frc</h2>
