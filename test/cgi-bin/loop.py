#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys
import time

# Activer le suivi des erreurs CGI pour voir les erreurs dans le navigateur
cgitb.enable()

# Entête HTTP indiquant que le contenu est HTML
print("Content-Type: text/html")
print()  # Ligne vide obligatoire après l'en-tête

# HTML de base pour la réponse
html = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Script CGI Python</title>
</head>
<body>
    <h1>Script CGI Python</h1>
"""

# Boucle infinie pour tester le timeout
html += "<h2>Boucle infinie</h2>\n"
html += "<p>Ce script CGI exécute une boucle infinie.</p>\n"
html += "<p>Le serveur devrait gérer un timeout côté serveur.</p>\n"
html += "<p>Pour observer le comportement, regardez le temps de réponse du serveur.</p>\n"
html += "<p>Arrêtez le script manuellement si nécessaire.</p>\n"

while True:
    html += "<p>Timestamp : {}</p>\n".format(time.strftime("%Y-%m-%d %H:%M:%S"))
    html += "<p>Attendez 1 seconde...</p>\n"
    sys.stdout.flush()
    time.sleep(1)

# Fin du HTML
html += """
</body>
</html>
"""

# Afficher la réponse HTML
print(html)
