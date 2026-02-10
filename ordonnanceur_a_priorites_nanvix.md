# Ordonnanceur à Priorités (Priority Scheduling)

## 1. Vue d'Ensemble

L'ordonnanceur implémenté est un système à **files de priorités multiniveaux**. Il ne traite pas tous les processus de la même manière : il favorise les tâches critiques du système (E/S, gestion mémoire) par rapport aux tâches utilisateurs.

À l'intérieur d'un même niveau de priorité, il applique une politique de **Round-Robin** afin de garantir l'équité entre les processus de priorité équivalente.

---

## 2. Structure des Priorités

Dans **Nanvix**, la priorité est définie par une valeur numérique dans la structure `process`. Contrairement à l'intuition, **plus la valeur est petite, plus la priorité est élevée**.

| Niveau de priorité | Constante   | Rôle |
|------------------|------------|------|
| Haute (-100)     | `PRIO_IO`   | Processus bloqués sur le disque (besoin urgent de CPU dès le retour d'une E/S). |
| Intermédiaire (0)| `PRIO_TTY`  | Interaction avec le clavier et l'écran (réactivité utilisateur). |
| Basse (40)       | `PRIO_USER` | Tâches de calcul standard et processus IDLE. |

---

## 3. Algorithme de Sélection (`yield`)

Lorsqu'un changement de contexte est nécessaire, la fonction `yield()` suit la logique suivante :

1. **Parcours de la table**  
   Le scheduler parcourt la table `proctab`, qui contient tous les processus du système.

2. **Filtrage**  
   Seuls les processus dans l'état `PROC_READY` sont éligibles à l'exécution.

3. **Comparaison de priorité**  
   - Le scheduler identifie la classe de priorité la plus haute disponible (la plus petite valeur de `p->priority`).  
   - Si un processus possède une priorité supérieure à celle du processus `next` actuellement sélectionné, il devient le nouveau candidat.

4. **Départage (Round-Robin)**  
   - Si deux processus ont la même priorité, le scheduler choisit celui dont le `counter` est le plus élevé.  
   - Le `counter` augmente à chaque tick passé en attente (mécanisme d'**aging**), garantissant qu'aucun processus ne soit affamé.

---

## 4. Cycle de Vie et Temps (Quantum)

Le temps processeur est découpé en tranches appelées **quanta**.

- **Quantum**  
  Chaque processus élu reçoit un crédit de **50 ticks** (`PROC_QUANTUM`).

- **Préemption temporelle**  
  Si un processus consomme entièrement son quantum :
  - son état repasse à `PROC_READY`,
  - son `counter` est remis à zéro par `sched()`,
  - il est replacé virtuellement en *fin de file* pour son niveau de priorité.

- **Blocage (E/S)**  
  Si un processus doit attendre une donnée (I/O) :
  - il abandonne immédiatement le CPU, même s'il lui reste du temps,
  - il passe à l'état `PROC_WAITING`,
  - il est retiré de la file de sélection jusqu'à son réveil.

