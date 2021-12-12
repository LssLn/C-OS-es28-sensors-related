# C-OS-es28-sensors-related
Ogni sensore è rappresentato da un processo/thread.
-Il primo sensore, periodicamente e ogni secondo, monitora la temperatura di una stanza.
-Il secondo sensore, periodicamente e ogni secondo, monitora l'umidità di una stanza.
-Il terzo sensore, periodicamente e ogni secondo, il consumo energetico.

tutti i sensori inviano i propri valori al main thread/processo padre che si occuperà di stamparli a video i valori dei tre sensori ogni 2 secondi

Dopo 20 secondi di esecuzione tutti i processi/thread sensori imposteranno il valore del sensore a -1 per comunicare al main thread7processo padre che hanno terminato.

Il processo padre/main thread, quando leggerà che il valore di tutti e tre i sensori è uguale a -1 termina.
