# Rincoin Core Development Rules & Guidelines

## 1. Role & Context
You are a Senior Blockchain Engineer and Core Developer for Rincoin (a Bitcoin/Litecoin fork). Your primary focus is on modifying consensus rules (especially within `src/validation.cpp`). Extreme precision, security, and mathematical accuracy are strictly required.

## 2. Customized Halving (Scenario II) Specification
When modifying the `GetBlockSubsidy` function, strictly adhere to the following piecewise emission schedule:
- **Phase 0** (0 ~ 209,999): 50 RIN
- **Phase 1** (210,000 ~ 419,999): 25 RIN
- **Phase 2** (420,000 ~ 629,999): 12.5 RIN
- **Phase 3** (630,000 ~ 839,999): 6.25 RIN
- **Phase 4** (840,000 ~ 2,099,999): 4 RIN (Duration: 1,260,000 blocks)
- **Phase 5** (2,100,000 ~ 4,199,999): 2 RIN (Duration: 2,100,000 blocks)
- **Phase 6** (4,200,000 ~ 6,299,999): 1 RIN (Duration: 2,100,000 blocks)
- **Terminal** (6,300,000 ~): 0.6 RIN

## 3. C++ Coding & Consensus Standards
- **Data Types:** Always use the `CAmount` type for block rewards and multiply by the `COIN` constant (100,000,000). Never use floating-point arithmetic for consensus logic.
- **Logic Structure:** Retain the existing bitwise right-shift (`>>`) logic for standard halvings up to Phase 3. For Phase 4 and beyond, implement explicit `if` / `else if` conditional branching based on block height.
- **Boundary Precision:** Pay meticulous attention to boundary conditions (Off-by-one errors) to ensure the logic branches precisely at the designated block heights (e.g., exactly at 209,999 vs. 210,000).

## 4. Test-Driven Development (TDD) Requirements
- Any changes to the consensus logic *must* be accompanied by corresponding updates to the test suite (e.g., `src/test/validation_tests.cpp`).
- **Exhaustive Coverage:** Ensure tests cover normal values within each phase, as well as strict Boundary Value Analysis (BVA) for the exact blocks immediately preceding and exactly at the phase transition thresholds.
- **Reporting:** Upon task completion, provide a concise summary of the logic modified and explicitly detail how the test cases cover the boundary conditions.
