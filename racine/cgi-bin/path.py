#!/usr/bin/env python3
import os

# Définir les headers HTTP
print("Content-Type: text/plain")

try:
    # Récupérer le chemin absolu de la ressource depuis PATH_INFO
    path_info = os.environ.get('PATH_INFO', '')
    script_dir = os.path.dirname(os.path.realpath(__file__))
    absolute_path = os.path.join(script_dir, path_info.lstrip('/'))

    # Lire le contenu du fichier
    with open(absolute_path, 'r') as file:
        content = file.read()

    # Calculer la longueur du contenu
    content_length = len(content)

    # Ajouter le header Content-Length
    print(f"Content-Length: {content_length}")
    print()  # Ligne vide pour séparer les headers du contenu

    # Afficher le contenu du fichier
    print(content)

except Exception as e:
    error_message = f"Erreur : {e}"
    print(f"Content-Length: {len(error_message)}")
    print()  # Ligne vide pour séparer les headers du contenu
    print(error_message)
