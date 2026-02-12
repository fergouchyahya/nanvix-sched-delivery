/*
 * Copyright(C) 2011-2016 Pedro H. Penna   <pedrohenriquepenna@gmail.com>
 *              2015-2016 Davidson Francis <davidsondfgl@hotmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License.
 */

#include <nanvix/clock.h>
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <signal.h>

/*
 * Place un processus dans l’état prêt.
 * Le compteur est remis à 0 pour forcer une replanification.
 */
PUBLIC void sched(struct process *proc)
{
	proc->state = PROC_READY;
	proc->counter = 0;
}

/*
 * Arrête le processus courant et notifie le père.
 * La main est immédiatement rendue à l’ordonnanceur.
 */
PUBLIC void stop(void)
{
	curr_proc->state = PROC_STOPPED;
	sndsig(curr_proc->father, SIGCHLD);
	yield();
}

/*
 * Reprend un processus précédemment arrêté.
 * On vérifie simplement son état.
 */
PUBLIC void resume(struct process *proc)
{
	if (proc->state == PROC_STOPPED)
		sched(proc);
}

/*
 * Sélectionne le prochain processus à exécuter.
 * Politique : favoriser celui qui a le plus faible ratio
 * (temps CPU utilisé / temps d’existence).
 */
PUBLIC void yield(void)
{
	struct process *p;
	struct process *next;
	unsigned ready_count;
	unsigned long long best_used;
	unsigned long long best_elapsed;

	/* Si le courant était en exécution, le remettre prêt */
	if (curr_proc->state == PROC_RUNNING)
		sched(curr_proc);

	last_proc = curr_proc;

	/* Gestion des alarmes */
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		if (!IS_VALID(p))
			continue;

		if ((p->alarm) && (p->alarm < ticks))
		{
			p->alarm = 0;
			sndsig(p, SIGALRM);
		}
	}

	/* Recherche du meilleur candidat */
	next = IDLE;
	ready_count = 0;
	best_used = 0;
	best_elapsed = 1;

	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		if (p->state != PROC_READY)
			continue;

		ready_count++;

		unsigned long long elapsed = ticks - p->creation_ticks;
		if (elapsed == 0)
			elapsed = 1; /* protection division */

		unsigned long long used =
			(unsigned long long)p->utime + p->ktime;

		/* Premier candidat */
		if (next == IDLE)
		{
			next = p;
			best_used = used;
			best_elapsed = elapsed;
			continue;
		}

		/*
		 * Comparaison sans virgule flottante :
		 * used/elapsed < best_used/best_elapsed
		 * ⇔ used * best_elapsed < best_used * elapsed
		 */
		if ((used * best_elapsed) < (best_used * elapsed))
		{
			next = p;
			best_used = used;
			best_elapsed = elapsed;
		}
	}

	/* Aucun processus prêt → idle */
	if (ready_count == 0)
		next = IDLE;

	next->priority = PRIO_USER;
	next->state = PROC_RUNNING;
	next->counter = PROC_QUANTUM;

	if (curr_proc != next)
		switch_to(next);
}
