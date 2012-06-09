/*
 * Dieses kleine Projekt dient dem Zweck, den Umgang mit einem 
 * STM32 Cortex M4 Mikrocontroller zu illustrieren. Es wird hierbei
 * bewusst auf die Verwendung der �blichen Libraries verzichtet, um
 * m�glichst wenig von der eigentlichen Vorgehensweise zu "verdecken".
 * Es geht hierbei nicht darum, mit m�glichst wenig Aufwand "das Ding
 * ans Laufen zu kriegen", sondern ein grundlegendes Verst�ndnis �ber
 * die Funktionsweise des M4 zu erlangen.
 *
 * Ben�tigte Dokumente:
 *  [1] RM0090 Reference Manual             (DM00031020.pdf)
 *  [2] STM32F405xx/STM32F407xx datasheet   (DM00037051.pdf)
 *
 *  Falls das STM32F4 discovery board genutzt wird:
 *  [3] UM1472 User Manual                  (DM00039084.pdf)
 *
 * Autor:  J. Kerdels 
 * Lizenz: CC BY 3.0
 */

// Reset and Clock Control
#include "rcc.h"

// Methoden f�r das STM32F4 discovery board
#include "discovery.h"

// Beispiele f�r das STM32F4 discovery board
#include "discovery_ex.h"

// In der Datei discovery_ex.c befinden sich #defines, die - wenn 
// einkommentiert - das entsprechende Beispiel ausw�hlen



int main(void) 
{
    // Zu Beginn der Verarbeitung muss die gew�nschte Taktung der
    // einzelnen Komponenten des Systems eingestellt werden. Dies
    // wird hier von der Methode rcc_init() in der Datei rcc.c 
    // erledigt.
    rcc_init();
    
    // Das STM32F4 discovery board bringt einige externe Komponenten
    // mit sich. Die Methode discovery_basic_init() richtet zun�chst
    // die Schnittstellen der einfacheren Komponenten (LEDs und Taster)
    // des STM32F4 ein.
    discovery_basic_init();

    //----------------------------------------------------------------------
    
    // In diesem einfachen Beispiel werden die 4 LEDs des discovery boards
    // eingeschaltet, wenn der User-Button des boards gedr�ckt wird.
    led_and_button_example();

    //----------------------------------------------------------------------
    
    // Dieses Beispiel zeigt, wie man die Grundkonfiguration eines Timers
    // durchf�hrt und wie man den Timer auf sehr einfach Weise nutzen kann.
    led_and_timer_example();
    
    //----------------------------------------------------------------------
    
    // In diesem Beispiel soll die Verwendung eines Interrupts erl�utert 
    // werden.
    timer_irq_example();

    //----------------------------------------------------------------------

    // In diesem Beispiel soll die Verwendung eines Timers zur PWM-Generierung
    // erl�utert werden.
    pwm_led_example();

    //----------------------------------------------------------------------

    // Das folgende Beispiel demonstriert, wie man mithilfe des DMA controllers
    // g�nzlich ohne Unterbrechung der Hauptprogrammschleife die LEDs weich 
    // ein- und ausblenden lassen kann.
    dma_pwm_led_example();


    return 0;
}