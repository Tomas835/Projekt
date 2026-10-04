# ZPG – Transformace, scény a vlastní logo

Projekt rozšiřuje původní aplikaci o základní transformace, více scén, les a vlastní 3D podpis.

## Splněné požadavky

1. **Základní transformace**
   - posun, změna měřítka a rotace kolem osy Z,
   - transformace jsou počítány ve vertex shaderu pomocí rovnic ze cvičení, bez matic.

2. **ShaderProgram – uniformy**
   - přetížené `setUniform()` pro `int`, `float`, 2/3/4 hodnoty,
   - před nastavením se kontroluje existence uniformní proměnné.

3. **Více scén**
   - třída `Scene` uchovává objekty scény,
   - přepínání scén klávesami `1–4`.

4. **Testovací scény**
   - `1` – trojúhelník,
   - `2` – kulička,
   - `3` – les,
   - `4` – vlastní 3D logo.

5. **Les**
   - 16 stromů,
   - 16 keřů,
   - stylizované slunce,
   - vlastní 3D logo.
   - Strom je záměrně zjednodušený na kmen a dvě trojúhelníkové části koruny.

6. **Main**
   - inicializace → shadery → modely → scény → hlavní smyčka.

7. **Vlastní 3D logo**
   - logo odpovídá přiloženému Blender souboru `Untitled.blend`,
   - text v Blenderu: **VYL0034**,
   - font: Arial,
   - extruze: `0.15`,
   - bevel: `0.01`,
   - geometrie je převedena na triangulovaný model v `models/vyl0034_logo.h`.

8. **Objektový návrh**
   - `Application`, `Scene`, `SceneObject`, `Model`, `ShaderProgram`,
   - přiložen je diagram `class_diagram.png`.

## Ovládání

- `1` – trojúhelník
- `2` – kulička
- `3` – les
- `4` – vlastní logo
- `ESC` – ukončení programu

## Spuštění

Otevřete `ukol.vcxproj` ve Visual Studiu a spusťte projekt. Projekt očekává knihovny GLAD, GLFW a GLM dostupné v prostředí cvičení; z odevzdávaného ZIPu jsou záměrně odstraněny, protože zadání říká knihovny neposílat.
