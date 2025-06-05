/**
 * @file kyber_crystal_manager.c
 * @brief Manages an array of Kyber crystals for a superlaser.
 *
 * This program simulates the core functionalities required to manage
 * a collection of Kyber crystals, essential components for a hypothetical
 * superlaser. It includes initialization, status checks, energizing sequences,
 * alignment procedures, and diagnostic routines.
 *
 * Technical Concepts:
 * - Data structures for complex components (Kyber Crystals).
 * - Enumerations for state management (CrystalStatus, AlignmentPrecision).
 * - Array-based management of multiple components.
 * - Control flow for sequential operations (energizing, aligning).
 * - Bitmasks for representing multiple fault conditions.
 * - Simulated hardware interaction and feedback.
 * - Error handling and reporting.
 *
 * Thematic Elements:
 * - Kyber Crystals: The core components being managed.
 * - Superlaser: The ultimate application of these crystals.
 * - Energizing/Alignment: Key steps before laser operation.
 * - Diagnostics: Ensuring operational readiness.
 */

#include <stdio.h>
#include <stdlib.h> // For rand(), srand()
#include <string.h> // For memset()
#include <stdbool.h> // For bool type
#include <time.h>   // For srand(time(NULL))

#define NUM_CRYSTALS 8 // Number of primary Kyber crystals in the array
#define MAX_ENERGY_LEVEL 1000 // Arbitrary maximum energy units per crystal
#define ENERGY_CHARGE_RATE 50  // Units of energy per charging cycle
#define ALIGNMENT_STEPS 5     // Number of steps to achieve full alignment

// Enumeration for crystal operational status
typedef enum {
    CRYSTAL_OFFLINE,
    CRYSTAL_NOMINAL,
    CRYSTAL_ENERGIZING,
    CRYSTAL_ENERGIZED,
    CRYSTAL_ALIGNING,
    CRYSTAL_ALIGNED,
    CRYSTAL_ACTIVE, // Ready for firing sequence
    CRYSTAL_FAULTY,
    CRYSTAL_DEPLETED
} CrystalStatus;

// Enumeration for alignment precision levels
typedef enum {
    ALIGN_NONE,
    ALIGN_COARSE,
    ALIGN_FINE,
    ALIGN_LOCKED
} AlignmentPrecision;

// Bitmasks for fault conditions (can be combined)
#define FAULT_NONE              0x00
#define FAULT_POWER_FLUCTUATION 0x01 // Bit 0
#define FAULT_CRACK_DETECTED    0x02 // Bit 1
#define FAULT_OVERHEATING       0x04 // Bit 2
#define FAULT_ALIGNMENT_DRIFT   0x08 // Bit 3

// Structure representing a single Kyber Crystal
typedef struct {
    int id;
    CrystalStatus status;
    unsigned int fault_codes; // Bitmask for multiple faults
    int current_energy_level;
    AlignmentPrecision alignment;
    int alignment_progress; // 0 to ALIGNMENT_STEPS
    char location_bay[20];  // e.g., "Focusing Chamber A1"
} KyberCrystal;

// Global array of Kyber crystals
KyberCrystal crystal_array[NUM_CRYSTALS];

// --- Helper Functions ---
const char* get_status_string(CrystalStatus status) {
    switch (status) {
        case CRYSTAL_OFFLINE: return "OFFLINE";
        case CRYSTAL_NOMINAL: return "NOMINAL (Idle)";
        case CRYSTAL_ENERGIZING: return "ENERGIZING";
        case CRYSTAL_ENERGIZED: return "ENERGIZED";
        case CRYSTAL_ALIGNING: return "ALIGNING";
        case CRYSTAL_ALIGNED: return "ALIGNED";
        case CRYSTAL_ACTIVE: return "ACTIVE (Ready)";
        case CRYSTAL_FAULTY: return "FAULTY";
        case CRYSTAL_DEPLETED: return "DEPLETED";
        default: return "UNKNOWN_STATUS";
    }
}

const char* get_alignment_string(AlignmentPrecision ap) {
    switch (ap) {
        case ALIGN_NONE: return "None";
        case ALIGN_COARSE: return "Coarse";
        case ALIGN_FINE: return "Fine";
        case ALIGN_LOCKED: return "Locked";
        default: return "UNKNOWN_ALIGN";
    }
}

void print_fault_codes(unsigned int faults) {
    if (faults == FAULT_NONE) {
        printf("None");
        return;
    }
    if (faults & FAULT_POWER_FLUCTUATION) printf("PowerFluctuation ");
    if (faults & FAULT_CRACK_DETECTED)    printf("CrackDetected ");
    if (faults & FAULT_OVERHEATING)       printf("Overheating ");
    if (faults & FAULT_ALIGNMENT_DRIFT)   printf("AlignmentDrift ");
}

// --- Crystal Management Functions ---

/**
 * @brief Initializes all Kyber crystals in the array to a default state.
 */
void initialize_crystal_array() {
    printf("Initializing Kyber Crystal Array (%d crystals)...\n", NUM_CRYSTALS);
    for (int i = 0; i < NUM_CRYSTALS; ++i) {
        crystal_array[i].id = i + 1;
        // Simulate some initial varied states; some might be offline or have minor faults
        if (rand() % 10 == 0) { // 10% chance of being initially faulty
             crystal_array[i].status = CRYSTAL_FAULTY;
             crystal_array[i].fault_codes = (1 << (rand() % 4)); // Random single fault
        } else if (rand() % 5 == 0) { // 20% chance of being offline
            crystal_array[i].status = CRYSTAL_OFFLINE;
            crystal_array[i].fault_codes = FAULT_NONE;
        }
        else {
            crystal_array[i].status = CRYSTAL_NOMINAL;
            crystal_array[i].fault_codes = FAULT_NONE;
        }
        crystal_array[i].current_energy_level = 0;
        crystal_array[i].alignment = ALIGN_NONE;
        crystal_array[i].alignment_progress = 0;
        sprintf(crystal_array[i].location_bay, "Sector %c, Bay %d", 'A' + (i / 4), (i % 4) + 1);
    }
    printf("Kyber Crystal Array initialization complete.\n");
}

/**
 * @brief Runs diagnostics on a specific crystal.
 * @param crystal_id The ID of the crystal to diagnose.
 * @return True if crystal is nominal or can be brought to nominal, false otherwise.
 */
bool diagnose_crystal(int crystal_id) {
    if (crystal_id <= 0 || crystal_id > NUM_CRYSTALS) {
        // fprintf(stderr, "Error: Invalid crystal ID %d for diagnosis.\n", crystal_id);
        return false;
    }
    KyberCrystal *crystal = &crystal_array[crystal_id - 1];
    printf("Diagnosing Crystal ID %d (%s):\n", crystal->id, crystal->location_bay);
    printf("  Status: %s, Energy: %d/%d, Alignment: %s (%d/%d steps)\n",
           get_status_string(crystal->status), crystal->current_energy_level, MAX_ENERGY_LEVEL,
           get_alignment_string(crystal->alignment), crystal->alignment_progress, ALIGNMENT_STEPS);
    printf("  Fault Codes: "); print_fault_codes(crystal->fault_codes); printf("\n");

    if (crystal->fault_codes != FAULT_NONE) {
        printf("  Action: Crystal has faults. Attempting to clear minor faults...\n");
        // Simulate attempting to clear faults. For this example, let's say power fluctuations can be stabilised.
        if (crystal->fault_codes & FAULT_POWER_FLUCTUATION) {
            printf("  Attempting to stabilize power for crystal %d...\n", crystal->id);
            // Simulate success/failure
            if (rand() % 2 == 0) {
                crystal->fault_codes &= ~FAULT_POWER_FLUCTUATION; // Clear the fault
                printf("  Power stabilized for crystal %d.\n", crystal->id);
            } else {
                printf("  Failed to stabilize power for crystal %d. Fault remains.\n", crystal->id);
            }
        }
        if (crystal->fault_codes != FAULT_NONE) {
             printf("  Crystal %d remains FAULTY after diagnostics.\n", crystal->id);
             crystal->status = CRYSTAL_FAULTY; // Ensure status reflects faults
             return false;
        } else {
             printf("  All clearable faults resolved for crystal %d. Status set to NOMINAL.\n", crystal->id);
             crystal->status = CRYSTAL_NOMINAL;
        }
    }
    if (crystal->status == CRYSTAL_OFFLINE) {
        printf("  Action: Crystal %d is OFFLINE. Attempting to bring online...\n", crystal->id);
        crystal->status = CRYSTAL_NOMINAL; // Simulate bringing online
        printf("  Crystal %d brought online. Status: NOMINAL.\n", crystal->id);
    }
    
    return crystal->status != CRYSTAL_FAULTY;
}

/**
 * @brief Initiates the energizing sequence for a specific crystal.
 * @param crystal_id The ID of the crystal to energize.
 * @return True if energizing process completed successfully, false otherwise.
 */
bool energize_crystal(int crystal_id) {
    if (crystal_id <= 0 || crystal_id > NUM_CRYSTALS) {
        // fprintf(stderr, "Error: Invalid crystal ID %d for energizing.\n", crystal_id);
        return false;
    }
    KyberCrystal *crystal = &crystal_array[crystal_id - 1];

    if (crystal->status == CRYSTAL_FAULTY) {
        printf("Crystal %d cannot be energized: FAULTY. Run diagnostics.\n", crystal->id);
        return false;
    }
    if (crystal->status == CRYSTAL_ENERGIZED || crystal->status == CRYSTAL_ACTIVE) {
        printf("Crystal %d is already ENERGIZED or ACTIVE.\n", crystal->id);
        return true;
    }
    if (crystal->status == CRYSTAL_OFFLINE) {
        printf("Crystal %d is OFFLINE. Please diagnose and bring online first.\n", crystal->id);
        return false;
    }


    printf("Initiating energizing sequence for Crystal ID %d (%s).\n", crystal->id, crystal->location_bay);
    crystal->status = CRYSTAL_ENERGIZING;

    while (crystal->current_energy_level < MAX_ENERGY_LEVEL) {
        // Simulate time passing and energy transfer
        // For a real system, this would involve hardware commands and feedback.
        // usleep(100000); // Simulate delay (requires unistd.h, platform dependent)
        for(volatile int i=0; i < 1000000; ++i); // Simple busy wait for simulation

        crystal->current_energy_level += ENERGY_CHARGE_RATE;
        if (crystal->current_energy_level > MAX_ENERGY_LEVEL) {
            crystal->current_energy_level = MAX_ENERGY_LEVEL;
        }
        printf("  Crystal %d: Energy at %d/%d units.\n", crystal->id, crystal->current_energy_level, MAX_ENERGY_LEVEL);

        // Simulate random fault during energizing
        if (rand() % 20 == 0) { // 5% chance of fault per cycle
            crystal->status = CRYSTAL_FAULTY;
            crystal->fault_codes |= FAULT_POWER_FLUCTUATION; // Example fault
            printf("  ERROR: Power fluctuation detected in Crystal %d during energizing! Sequence aborted.\n", crystal->id);
            return false;
        }
    }

    crystal->status = CRYSTAL_ENERGIZED;
    printf("Crystal %d successfully ENERGIZED to %d units.\n", crystal->id, crystal->current_energy_level);
    return true;
}

/**
 * @brief Initiates the alignment sequence for a specific crystal.
 * @param crystal_id The ID of the crystal to align.
 * @return True if alignment process completed successfully, false otherwise.
 */
bool align_crystal(int crystal_id) {
    if (crystal_id <= 0 || crystal_id > NUM_CRYSTALS) {
        // fprintf(stderr, "Error: Invalid crystal ID %d for alignment.\n", crystal_id);
        return false;
    }
    KyberCrystal *crystal = &crystal_array[crystal_id - 1];

    if (crystal->status == CRYSTAL_FAULTY) {
        printf("Crystal %d cannot be aligned: FAULTY.\n", crystal->id);
        return false;
    }
    if (crystal->status != CRYSTAL_ENERGIZED && crystal->status != CRYSTAL_ALIGNED && crystal->status != CRYSTAL_ACTIVE) {
        printf("Crystal %d must be ENERGIZED before alignment. Current status: %s.\n", crystal->id, get_status_string(crystal->status));
        return false;
    }
     if (crystal->status == CRYSTAL_ALIGNED || crystal->status == CRYSTAL_ACTIVE) {
        printf("Crystal %d is already ALIGNED or ACTIVE.\n", crystal->id);
        return true;
    }


    printf("Initiating alignment sequence for Crystal ID %d (%s).\n", crystal->id, crystal->location_bay);
    crystal->status = CRYSTAL_ALIGNING;
    crystal->alignment_progress = 0;
    crystal->alignment = ALIGN_COARSE;

    for (int step = 0; step < ALIGNMENT_STEPS; ++step) {
        // Simulate alignment mechanism
        // usleep(150000); // Simulate delay
        for(volatile int i=0; i < 1500000; ++i); // Simple busy wait

        crystal->alignment_progress++;
        if (crystal->alignment_progress >= ALIGNMENT_STEPS * 0.75) { // Last 25% is fine tuning
            crystal->alignment = ALIGN_FINE;
        }
        printf("  Crystal %d: Alignment progress %d/%d. Precision: %s.\n",
               crystal->id, crystal->alignment_progress, ALIGNMENT_STEPS, get_alignment_string(crystal->alignment));

        // Simulate random alignment drift or error
        if (rand() % 25 == 0) { // 4% chance of alignment issue
            crystal->status = CRYSTAL_FAULTY;
            crystal->fault_codes |= FAULT_ALIGNMENT_DRIFT;
            printf("  ERROR: Alignment drift detected in Crystal %d! Sequence aborted.\n", crystal->id);
            crystal->alignment = ALIGN_NONE;
            crystal->alignment_progress = 0;
            return false;
        }
    }

    crystal->alignment = ALIGN_LOCKED;
    crystal->status = CRYSTAL_ALIGNED;
    printf("Crystal %d successfully ALIGNED. Precision: %s.\n", crystal->id, get_alignment_string(crystal->alignment));
    return true;
}

/**
 * @brief Checks the overall status of the Kyber crystal array.
 * @param required_ready Minimum number of crystals that need to be ACTIVE.
 * @return True if the array meets the minimum readiness criteria, false otherwise.
 */
bool check_array_readiness(int required_ready) {
    int active_crystals = 0;
    printf("\n--- Kyber Crystal Array Status Report ---\n");
    for (int i = 0; i < NUM_CRYSTALS; ++i) {
        KyberCrystal *c = &crystal_array[i];
        printf("Crystal ID %2d (%-18s): Status=%-15s Energy=%4d/%-4d Align=%-6s (%d/%d) Faults: ",
               c->id, c->location_bay, get_status_string(c->status), c->current_energy_level, MAX_ENERGY_LEVEL,
               get_alignment_string(c->alignment), c->alignment_progress, ALIGNMENT_STEPS);
        print_fault_codes(c->fault_codes);
        printf("\n");

        if (c->status == CRYSTAL_ACTIVE) {
            active_crystals++;
        } else if (c->status == CRYSTAL_ALIGNED) { // If aligned but not active, try to set active
            c->status = CRYSTAL_ACTIVE; // Transition to active if fully prepped
            printf("  Crystal %d transitioned to ACTIVE state.\n", c->id);
            active_crystals++;
        }
    }
    printf("---------------------------------------\n");
    printf("Total ACTIVE crystals: %d / %d (Required: %d)\n", active_crystals, NUM_CRYSTALS, required_ready);
    return active_crystals >= required_ready;
}

/**
 * @brief Prepares the entire Kyber array for a firing sequence.
 * Attempts to diagnose, energize, and align all non-faulty crystals.
 * @param min_crystals_for_firing Minimum number of crystals needed to be active.
 * @return True if the array is successfully prepared, false otherwise.
 */
bool prepare_array_for_firing(int min_crystals_for_firing) {
    printf("\n===== Initiating Superlaser Array Preparation Sequence =====\n");
    int successfully_prepared_crystals = 0;

    // Step 1: Diagnose all crystals
    printf("\n--- Phase 1: Diagnostics ---\n");
    for (int i = 0; i < NUM_CRYSTALS; ++i) {
        diagnose_crystal(crystal_array[i].id);
        // Brief pause for realism
        for(volatile int k=0; k < 500000; ++k);
    }

    // Step 2: Energize all nominal crystals
    printf("\n--- Phase 2: Energizing Sequence ---\n");
    for (int i = 0; i < NUM_CRYSTALS; ++i) {
        if (crystal_array[i].status == CRYSTAL_NOMINAL || crystal_array[i].status == CRYSTAL_DEPLETED) {
            if (!energize_crystal(crystal_array[i].id)) {
                printf("  Crystal %d failed to energize. Will not proceed with this crystal.\n", crystal_array[i].id);
            }
        } else if (crystal_array[i].status == CRYSTAL_ENERGIZED) {
             printf("  Crystal %d already energized.\n", crystal_array[i].id);
        }
         else {
            printf("  Skipping energizing for Crystal %d (Status: %s).\n", crystal_array[i].id, get_status_string(crystal_array[i].status));
        }
    }

    // Step 3: Align all energized crystals
    printf("\n--- Phase 3: Alignment Sequence ---\n");
    for (int i = 0; i < NUM_CRYSTALS; ++i) {
        if (crystal_array[i].status == CRYSTAL_ENERGIZED) {
            if (!align_crystal(crystal_array[i].id)) {
                printf("  Crystal %d failed to align. Will not be active.\n", crystal_array[i].id);
            }
        } else if (crystal_array[i].status == CRYSTAL_ALIGNED) {
             printf("  Crystal %d already aligned.\n", crystal_array[i].id);
        }
         else {
             printf("  Skipping alignment for Crystal %d (Status: %s).\n", crystal_array[i].id, get_status_string(crystal_array[i].status));
        }
        if(crystal_array[i].status == CRYSTAL_ALIGNED){ // If successfully aligned, mark active
            crystal_array[i].status = CRYSTAL_ACTIVE;
            successfully_prepared_crystals++;
        }
    }
    
    printf("\n===== Superlaser Array Preparation Sequence Complete =====\n");
    return check_array_readiness(min_crystals_for_firing);
}


int main() {
    srand(time(NULL)); // Seed random number generator

    printf("Death Star Kyber Crystal Management System Booting Up...\n");
    printf("--------------------------------------------------------\n");

    initialize_crystal_array();
    check_array_readiness(NUM_CRYSTALS); // Initial check

    // Simulate an attempt to prepare for firing
    // Require at least 75% of crystals to be ready for this test firing
    int required_for_test_fire = (NUM_CRYSTALS * 3) / 4;
    if (required_for_test_fire == 0 && NUM_CRYSTALS > 0) required_for_test_fire = 1;


    if (prepare_array_for_firing(required_for_test_fire)) {
        printf("\nSUCCESS: Kyber Crystal Array is PREPARED for test firing sequence. %d crystals active.\n", required_for_test_fire);
        // TODO: Trigger simulated firing sequence
    } else {
        printf("\nFAILURE: Kyber Crystal Array NOT ready for test firing. Insufficient active crystals.\n");
        printf("Further diagnostics and manual intervention may be required by Imperial Engineers.\n");
    }
    
    // Example: Manually try to fix and re-prepare a specific crystal if one failed
    // For instance, if crystal_array[0] is faulty.
    if (crystal_array[0].status == CRYSTAL_FAULTY) {
        printf("\nAttempting manual intervention for Crystal 1 (%s)...\n", crystal_array[0].location_bay);
        if(diagnose_crystal(1)) { // Try to diagnose and fix
            if(energize_crystal(1)) {
                if(align_crystal(1)) {
                    crystal_array[0].status = CRYSTAL_ACTIVE;
                    printf("Crystal 1 successfully brought to ACTIVE state after manual intervention.\n");
                }
            }
        }
        check_array_readiness(required_for_test_fire); // Check status again
    }


    printf("\nKyber Crystal Management System Shutting Down...\n");
    printf("--------------------------------------------------------\n");

    return 0;
}

/**
 * Potential future enhancements:
 * 1. More Complex Fault Handling: Specific recovery procedures for different faults.
 * 2. Energy Decay Simulation: Crystals lose energy over time if not used.
 * 3. Dynamic Recalibration: Crystals requiring realignment after firing or over time.
 * 4. Inter-Crystal Dependencies: Some crystals might depend on others being online first.
 * 5. Command Interface: A simple text-based command parser to interact with the system (e.g., "diagnose 3", "energize_all").
 * 6. Logging: Output status and errors to a log file.
 * 7. Multi-threading: Simulate concurrent operations (e.g., energizing multiple crystals in parallel) - complex.
 * 8. Firing Sequence Logic: Detailed steps for combining crystal beams, aiming, and discharging.
 */
