## Použitie Markovho generátora hesiel

### Parametre

```
  -S, --statistics       súbor obsahujúci štatistiky
  -T, --thresholds=glob[:pos, [pos]]
                         počet znakov použitých na každej pozícii v hesle
          - glob - globálna hodnota pre všetky pozície (defaultne 95)
          - pos - hodnoty pre každú pozíciu zvlášť (prepíšu globálne hodnoty)
  -L, --length=min:max   dĺžka generovaného hesla (defaultne 1:50)
  -M, --mask             maska
  -X, --model            typ Markovského modelu
          - classic - Markov model prvého rádu (defaultný)
          - layered - Vrstvový Markov model
  -I - vráti heslo zodpovedajúce danému indexu (použitie len pre experimenty)
  -C, --cpu-generator - použiť CPU generátor namiesto GPU (funguje len s týmto
                        typom útoku)
```

### Súbor so štatistikami

Súbor obsahujúci štatistiky pre klasický alebo vrstvový Markov model
(aj obidve súčasne), ktorý bol vytvorený pomocou nástroja wstatgen vytvoreného pre
tieto účely. V prípade, že súbor neobsahuje štatistiky pre zadaný parameter
modelu, program ohlási chybu.

Tento parameter je povinný a nie je možné bez neho generovať heslá!

### Nastavenie prahu

Hodnotu prahu je možné špecifikovať pre všetky pozície súčasne a následne aj
pre každú pozíciu zvlášť postupne od prvej pozície v hesle. Jednotlivé hodnoty
sa oddeľujú čiarkou. Pre nešpecifikované pozície zostáva globálna hodnota.

### Použitie masky

Syntax masky je rovnaká ako v prípade nástroja
[oclHashcat](https://hashcat.net/oclhashcat/) a
[JohnTheRipper](http://www.openwall.com/john/).
Pri hodnote masky je možné použiť následovné zástupné znaky (metaznaky).

- `?l` - malé písmená (`abcdefghijklmnopqrstuvwxyz`)
- `?u` - veľké písmená (`ABCDEFGHIJKLMNOPQRSTUVWXYZ`)
- `?d` - číslice (`0123456789`)
- `?s` - špeciálne symboly (`<<medzera>>!"#$%&'()*+,-./:;<=>?@[]^_{|}~`)
- `?a` - všetky predchádzajúce súčasne (`?l?u?d?s`)

Okrem metaznakov môžeme špecifikovať pre niektoré pozície aj jedej jediný znak.
K tomu postačuje tento znak zapísať priamo (bez ?), výnimku tvorí znak ?,
ktorý je potrebné zduplikovať (??).

Napr. následujúca maska špecifikuje generovanie hesiel s počiatočným znakom A,
 po ktorom následujú malé písmená až po dĺžku 5.
 Pre vyššie dĺžky zostávajú používané znaky nezmenené,
 t.j. v závislosti na pravdepodobnosti a hodnote prahu.

```
A?l?l?l?l
```

Na základe zadanej masky sa patrične znížia hodnoty prahu podľa počtu znakov
špecifikovaných metaznakom. Ak je hodnota prahu nižšia ako počet týchto znakov,
zostáva nezmenená a pre generovanie je použitých len tento počet znakov v závislosti na ich pravdepodobnosti.

Použitím pozičnej masky dokonca môžeme používať ešte väčšie množstvo znakov, ako zodpovedajú metaznaku. Napr. ak špecifikujeme maskou na niektorej pozícii číslice, ale zároveň stanovíme
pozičnú hodnotu prahu vyššiu ako počet všetkých číslic, použiju sa aj iné znaky v závislosti na ich pravdepodobnosti výskytu.
