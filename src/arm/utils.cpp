#include <hw_rng.h>
#include "utils.h"


// void generateRandomValue() {
//     uint32_t randomValue = SimpleHacks::HW_RNG::get_uint32();
//     // Example: Use randomValue (e.g., Serial.println(randomValue);)
// }

uint32_t getRandomNumber32() {
    return SimpleHacks::HW_RNG::get_uint32(); // Return a random 32-bit number
}