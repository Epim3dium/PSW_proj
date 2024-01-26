# Lista o ograniczonej pojemności (12 pkt)
Zaimplementuj strukturę danych typu lista, przechowującą wskaźniki na dostarczone
przez użytkownika obszary pamięci (np. łańcuchy tekstowe). Lista może się dynamicz-
nie rozrastać i kurczyć w zależności od sekwencji wywołań wewnętrznych operacji,
ale jej całkowita pojemność nie może przekroczyć zadanego N . Lista powinna umożli-
wiać bezpieczne, współbieżne przetwarzanie wielowątkowe: dowolna liczba wątków
może w dowolnym momencie wywoływać dowolne funkcje listy. Poniżej znajduje się
lista operacji realizowanych przez listę:
```
void put(void *el)
```
operacja dodawania elementu el (reprezentowanego wskaźnikiem) na koniec
listy. Operacja może być blokująca, jeżeli lista zawiera już N lub więcej elemen-
tów.
```
void* get()
```
operacja usuwania pierwszego (najstarszego) elementu z listy. Wskaźnik do usu-
wanego elementu jest zwracany przez funkcję. Funkcja może być blokująca je-
żeli lista jest pusta.
D Zadania projektowe 57
```
int remove(void *el)
```
operacja usuwania z listy elementu el.
```
int getCount()
```
operacja zwracania aktualnej liczby elementów na liście.
```
void setMaxSize(int n)
```
operacja ustalania nowego, maksymalnego rozmiaru listy. Jeżeli nowy rozmiar
jest mniejszy od aktualnej liczby elementów na liście, to zachowywane są ele-
menty nadmiarowe, ale dodawanie nowych będzie możliwe dopiero po osiągnię-
ciu pojemności mniejszej od nowego maksymalnego rozmiaru.
# Publish-subscribe (15 pkt)
Zaimplementuj system dystrybucji wiadomości typu publish-subscribe o ograniczonej
do N wiadomości pojemności. Dowolny wątek może zasubskrybować kolejkę i od tego
momentu będzie miał możliwość pobierania ich z kolejki. Każda wiadomość, zanim zo-
stanie usunięta, musi być odczytana/odebrana przez każdego zasubskrybowanego – w
momencie wysyłania wiadomości – wątku. W systemie może być przechowywanych
maksymalnie N wiadomości. Próba dostarczenia wiadomości N + 1 skutkuje zabloko-
waniem wątku wysyłającego. Odbiór kolejnej wiadomości również może być blokują-
cy, jeżeli wątek odczytał już wszystkie wiadomości z kolejki. Dostarczanie wiadomości
do kolejki bez subskrybentów, powoduje natychmiastowe usuwanie tych wiadomości.
Interfejs systemu powinien składać się z następujących funkcji:
```
createQueue(TQueue *queue, int *size)
```
inicjuje strukturę TQueue reprezentującą nową kolejkę o początkowym, maksy-
malnym rozmiarze size.
```
destroyQueue(TQueue *queue)
```
usuwa kolejkę queue i zwalnia pamięć przez nią zajmowaną. Próba dostarczania
lub odbioru nowych wiadomości z takiej kolejki będzie kończyła się błędem.
```
subscribe(TQueue *queue, pthread_t *thread)
```
rejestruje wątek thread jako kolejnego odbiorcę wiadomości z kolejki queue.
```
unsubscribe(TQueue *queue, pthread_t *thread)
```
wyrejestrowuje wątek thread z kolejki queue. Nieodebrane przez wątek wiado-
mości są traktowane jako odebrane.
```
put(TQueue *queue, void *msg)
```
wstawia do kolejki queue nową wiadomość reprezentowaną wskaźnikiem msg.
```
void* get(TQueue *queue, pthread_t *thread)
```
odbiera pojedynczą wiadomość z kolejki queue dla wątku thread. Jeżeli nie ma
nowych wiadomości, funkcja jest blokująca. Jeżeli wątek thread nie jest zasub-
skrybowany – zwracany jest pusty wskaźnik NULL.
D Zadania projektowe 58
```
getAvailable(TQueue *queue, pthread_t *thread)
```
zwraca liczbę wiadomości z kolejki queue dostępnych dla wątku thread.
```
remove(TQueue *queue, void *msg)
```
usuwa wiadomość msg z kolejki.
```
setSize(TQueue *queue, int *size)
```
ustala nowy, maksymalny rozmiar kolejki. Jeżeli nowy rozmiar jest mniejszy od
aktualnej liczby wiadomości w kolejce, to nadmiarowe wiadomości są usuwane
z kolejki, począwszy od najstarszych.
