# OpenGL Game Engine / Lesní bloudění

## Zadání

Cílem mého projektu bylo vytvořit jednoduchý herní engine, který simuluje základní herní funkcionalitu.

Můj projekt generuje procedurální terén pomocí Perlin Noise a na ten následně dle předem dané mapy generuje a instančně renderuje objekty lesa - stromy, větve, kameny, houby atd. Tyto objekty se generují po chuncích vůči poloze hráče. Texturování terénu je vytvořeno pomocí multitextury, která je taktéž čtena z mapy, která každé barvě přiřazuje jednu texturu. Tyto dílčí textury jsou tvořeny PBR materiály. Takto tvořená voda se dokonce i pohybuje.

Scéna podporuje i denní cyklus. Dle toho je v danou chvíli osvětlena buď měsícem nebo sluncem. Měsíc se chová jako bodové světlo, zatímco slunce jako směrové. Dle denní doby je definována i hustota a barva mlhy, která se následně propočítává v jednotlivých shaderech.

Pokud ani to nestačí, implementace podporuje i baterku ve formě reflektoru a nebo louče, které zde jako bodová světla umístit do scény. Postava hráče je animována, na načtení modelu je použit skinning. Projekt podporuje i základní pohyb pomocí šipek a rozhlížení pomocí myši. Kamera má celkem 4 módy - první osoba z pohledu hráče, třetí osoba na hráče, z vrchu, z vrchu s pohybem po křivce. Ty mezi sebou umí plynule přecházet. Se scénou dále lze interagovat.

Dohromady tvoří jednoduchý lesní celek, který umožňuje hráči se v něm pohybovat a při každém zapnutí generuje jiný prostor.

## Ovládání

Pohyb myši - rozhlížení

Kliknutí na strom - změna modelu daného stromu

Šipky - pohyb hráče

R - restart scény

V - změna pohledu kamery

C - zapnutí/vypnutí kurzoru v první osobě

O - defaultní pitch kamery ve třetí osobě

P - defaultní vzdálenost kamery ve třetí osobě

L - zapnutí/vypnutí baterky

T - postavení louče na místě Kliknutí

B - změna postavičky

TAB - zapnutí/vypnutí denního cyklus

ESC - opuštění aplikace 
