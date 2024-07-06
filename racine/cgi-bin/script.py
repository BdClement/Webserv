#!/usr/bin/env python3
import cgi
import cgitb
import os
import sys

# Activer le suivi des erreurs CGI pour voir les erreurs dans le navigateur
cgitb.enable()



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

# Si la méthode est GET
if "REQUEST_METHOD" in os.environ and os.environ["REQUEST_METHOD"] == "GET":
    html += """
    <h2>Requête GET</h2>
    <p>Pas de données à afficher pour une requête GET.</p>
    """

# Si la méthode est POST
elif "REQUEST_METHOD" in os.environ and os.environ["REQUEST_METHOD"] == "POST":
    html += """
    <h2>Requête POST</h2>
    <p>Données reçues :</p>
    <ul>
    """
    # Lire la longueur du contenu
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    if content_length > 0:
        # Lire les données depuis stdin
        post_data = sys.stdin.read(content_length)
        html += f"<li><b>Raw POST Data</b>: {post_data}</li>\n"

    html += """
    </ul>
    """

# Si la méthode HTTP n'est ni GET ni POST
else:
    html += """
    <h2>Méthode HTTP non supportée</h2>
    <p>Ce script CGI ne supporte que les requêtes GET et POST.</p>
    """

# Fin du HTML
html += """
</body>
</html>
"""

# Calcul de la longueur du contenu généré
content_length = len(html.encode('utf-8'))

# Entête HTTP indiquant que le contenu est HTML et la longueur du contenu
print("Content-Type: text/html")
print(f"Content-Length: {content_length}")
print()  # Ligne vide obligatoire après les en-têtes

# Afficher la réponse HTML
print(html)
