#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <stdbool.h>
#include <stdint.h>

// Element identifiers
typedef enum {
    ELEMENT_NONE  = 0,
    ELEMENT_LIGHT = (1 << 0),
    ELEMENT_VOID  = (1 << 1),
    ELEMENT_WATER = (1 << 2),
    ELEMENT_FIRE  = (1 << 3),
    ELEMENT_EARTH = (1 << 4),
    ELEMENT_AIR   = (1 << 5),
    ELEMENT_FROST = (1 << 6)
} Element;

// Bitwise masks defining which elements each element counters
#define ELEMENT_LIGHT_BEATS (ELEMENT_VOID | ELEMENT_EARTH | ELEMENT_FIRE)
#define ELEMENT_VOID_BEATS  (ELEMENT_EARTH | ELEMENT_FROST | ELEMENT_WATER)
#define ELEMENT_WATER_BEATS (ELEMENT_FIRE | ELEMENT_FROST | ELEMENT_LIGHT)
#define ELEMENT_FIRE_BEATS  (ELEMENT_AIR | ELEMENT_FROST | ELEMENT_VOID)
#define ELEMENT_EARTH_BEATS (ELEMENT_AIR | ELEMENT_FIRE | ELEMENT_WATER)
#define ELEMENT_AIR_BEATS   (ELEMENT_LIGHT | ELEMENT_VOID | ELEMENT_WATER)
#define ELEMENT_FROST_BEATS (ELEMENT_EARTH | ELEMENT_AIR | ELEMENT_LIGHT)

/**
 * Gets the bitmask of all elements that an element beats.
 */
static inline uint32_t element_get_advantages(Element elem) {
    switch (elem) {
        case ELEMENT_LIGHT: return ELEMENT_LIGHT_BEATS;
        case ELEMENT_VOID:  return ELEMENT_VOID_BEATS;
        case ELEMENT_WATER: return ELEMENT_WATER_BEATS;
        case ELEMENT_FIRE:  return ELEMENT_FIRE_BEATS;
        case ELEMENT_EARTH: return ELEMENT_EARTH_BEATS;
        case ELEMENT_AIR:   return ELEMENT_AIR_BEATS;
        case ELEMENT_FROST: return ELEMENT_FROST_BEATS;
        default:            return 0;
    }
}

/**
 * Checks if elem_a beats elem_b.
 */
static inline bool element_beats(Element elem_a, Element elem_b) {
    return (element_get_advantages(elem_a) & elem_b) != 0;
}

/**
 * Compares two elements.
 * 
 * @param element0 The primary element.
 * @param element1 The target element.
 * @return  1 if element0 beats element1,
 *         -1 if element1 beats element0,
 *          0 if they are equal or neither beats the other.
 */
static inline int element_compare(Element element0, Element element1) {
    if (element0 == element1) {
        return 0;
    }

    bool e0_beats_e1 = element_beats(element0, element1);
    bool e1_beats_e0 = element_beats(element1, element0);

    if (e0_beats_e1) return 1;
    if (e1_beats_e0) return -1;
    
    return 0;
}

/**
 * Helper to get a human-readable string name for an element.
 */
static inline const char* element_to_string(Element elem) {
    switch (elem) {
        case ELEMENT_LIGHT: return "Light";
        case ELEMENT_VOID:  return "Void";
        case ELEMENT_WATER: return "Water";
        case ELEMENT_FIRE:  return "Fire";
        case ELEMENT_EARTH: return "Earth";
        case ELEMENT_AIR:   return "Air";
        case ELEMENT_FROST: return "Frost";
        default:            return "Unknown";
    }
}

#endif // ELEMENTS_H