# HybridBranchPredictor
A simple hybrid branch predictor implementation for ChampSim that combines gshare and a local branch predictor. 

Designed as a project for the subject "Ampliación de Estructura de Computadores" 2024-2025. Facultad de Informática de la Universidad de Murcia.

A hybrid predictor will be implemented, combining gshare (global history) with a local branch predictor (LBP), where the history of each branch is considered independently.

The local branch predictor has been used in various generations of commercial processors, such as Intel's Pentium MMX, Pentium II, and Pentium III, which employed 4-bit histories and tables with 16 entries per branch.

In this implementation, we use 1024 entries for both the global prediction table and the local prediction table. A combination of both predictors (LBP and gshare) along with a meta-predictor will be used to decide which predictor is correct for each branch. The meta-predictor will implement a 2-bit counter for each position in an array of size 1024.

The meta-predictor update will follow these rules:

    The meta-predictor will be updated and accessed using the global index (for better performance).
    Local branch predictor (LBP), hereafter P1.
    Gshare predictor, hereafter P2.
    P1 and P2 both mispredict → No action is taken on the counter.
    P1 mispredicts and P2 predicts correctly → Counter is incremented (if not at MAX).
    P1 predicts correctly and P2 mispredicts → Counter is decremented (if not at MIN).
    P1 and P2 both predict correctly → No action is taken on the counter.

The prediction decision is based on the counter value:

    If COUNTER = 00 → Use P1.
    If COUNTER = 01 → Use P1.
    If COUNTER = 10 → Use P2.
    If COUNTER = 11 → Use P2.

Where MAX = 3 and MIN = 0.

To determine how to initialize the meta-predictor, a trial-and-error method was applied. The conclusion is that initializing the counter to 01 (i.e., using P1) provides the best average performance across all scenarios.

