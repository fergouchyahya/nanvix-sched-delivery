# Explication détaillée – Scheduler garanti et tests d’équité

## Logique du scheduler garanti
L’objectif est de donner à chaque processus un temps CPU proportionnel à sa « part » théorique depuis sa création.
- Chaque processus mémorise le tick où il est né (`creation_ticks`).
- À chaque décision :
  - `active` = nombre de processus READY/RUNNING (hors IDLE), minimal 1.
  - `elapsed = max(1, ticks - creation_ticks)` pour ce processus.
  - `cpu_used = utime + ktime` (temps user + kernel consommé).
  - Score à minimiser : `(cpu_used * active) / elapsed` (comparaison par produit croisé, sans flottant).
  - Le quantum reste `PROC_QUANTUM`; on supprime les anciens compteurs d’attente.
  - Alarms et transitions d’état restent inchangées.

## Modifications prévues fichier par fichier
- `nanvix/include/pm.h` : ajouter `creation_ticks` (ou `born`) dans `struct process` pour garder l’horodatage de naissance; éventuellement un cache de ratio.
- `nanvix/src/kernel/pm/pm.c` : initialiser `creation_ticks = ticks` pour IDLE (et tout task noyau créé à la main) afin d’éviter des valeurs indéfinies.
- `nanvix/src/kernel/pm/fork.c` : après attribution du PID, fixer `child->creation_ticks = ticks`; ne pas hériter de la valeur du parent.
- `nanvix/src/kernel/pm/sched.c` : remplacer la boucle de sélection par le calcul du score ci‑dessus; conserver `counter = PROC_QUANTUM` pour le vainqueur; enlever l’ancienne logique de wait‑counter; garder les alarmes.
- `nanvix/src/kernel/pm/clock.c` : aucune logique nouvelle requise (utime/ktime sont déjà incrémentés à chaque tick); toucher seulement car besoin d’un helper pour lire `ticks`.
- `nanvix/src/sbin/test/test.c` : tests utilisateur pour vérifier l’équité observée.

## Ce qui est fait dans `test.c`
Tests d’équité simples basés sur `times()` :
- **sched_test4** : 2 processus CPU‑bound, chacun écrit son `tms_utime` dans un fichier temp `sched4.tmp`. Le parent lit, calcule moyenne/différence et réussit si `|u0 - u1| ≤ 25%` de la moyenne.
- **sched_test5** : 3 processus, même logique avec `sched5.tmp`; réussite si chaque `|ui - avg| ≤ 30%`.
- Charge CPU déterministe : chaque enfant lance `work_cpu()` trois fois pour accumuler quelques ticks.
- Nettoyage : les fichiers temporaires sont supprimés après lecture.
- En mode VERBOSE, les moyennes et écarts sont affichés pour diagnostiquer.

## Pourquoi cette approche
- Le score `(cpu_used * active) / elapsed` est la version discrète d’un scheduler « guaranteed » : un processus est favorisé s’il a consommé peu de CPU au regard de son âge et du nombre d’actifs.
- Les tests écriture/lecture fichier remplacent les pipes (moins robustes ici) pour mesurer l’équité sans bruit d’E/S.
- Les seuils 25% / 30% sont assez stricts pour détecter des déséquilibres tout en tolérant la granularité de `times()`.

## Comment valider
1. Appliquer les modifications noyau décrites ci‑dessus.
2. Attendre que les tests « fair 2 procs » et « fair 3 procs » passent sans écart excessif ; en VERBOSE, vérifier que les diffs sont dans les bornes.

## Rappel des fichiers concernés
- `nanvix/include/pm.h`
- `nanvix/src/kernel/pm/pm.c`
- `nanvix/src/kernel/sys/fork.c`
- `nanvix/src/kernel/pm/sched.c` 
- `nanvix/src/sbin/test/test.c`


