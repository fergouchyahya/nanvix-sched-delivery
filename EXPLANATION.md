# Guaranteed Scheduling — Explanation (Nanvix)

**Author:** Fergouch Yahya  
**Branch:** guaranteed-scheduling-fergouch-yahya

## 1. Goal
Describe what "Guaranteed Scheduling" should guarantee in this project:
- what fairness/guarantee is expected
- what metric is used (CPU share, virtual runtime, etc.)
- what assumptions are made (processes, quantum, accounting)

## 2. Core idea
Explain the algorithm in simple terms:
- what value you compute for each process
- how you choose the next process
- how you update accounting after running

## 3. Data and bookkeeping
List the data used for scheduling decisions:
- which Nanvix fields are used (e.g., utime, ktime, priority, counter, etc.)
- any added fields (if any)
- where they are updated

## 4. Scheduling decision rule
Write the decision rule precisely (pseudo-code or formula).

## 5. Tests
Explain the test strategy:
- what behavior is verified
- what workloads/scenarios are used
- pass/fail criteria

## 6. Files delivered
This section will list the full files included in this branch once finalized.
