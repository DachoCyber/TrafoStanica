#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <tuple>
#include <map>
#include <queue>

std::map<int, std::vector<int>> graf;
std::map<int, float> strujeStubova; // Struja koja prolazi kroz stubove
std::map<std::pair<int, int>, float> strujaKrozVod; // Struja kroz svaki vod

struct Stub {
    int id;
    double ulaznaStruja;

    bool operator<(const Stub& other) const {
        return std::tie(id, ulaznaStruja) < std::tie(other.id, other.ulaznaStruja);
    }

    bool operator==(const Stub& other) const {
        return id == other.id;
    }
};

struct Vod {
    int id;
    std::vector<Stub> stubovi;
};

class Potrosac {
public:
    Stub stub;
    std::string ime;
    Vod vod;
    float struja;
};

void dodajUVod(int trBrPotrosaca, std::vector<Potrosac>& potrosaci, std::vector<Vod>& vodovi) {
    if (trBrPotrosaca < 0) return;
    int idVod = potrosaci[trBrPotrosaca].vod.id;
    Stub stub = potrosaci[trBrPotrosaca].stub;
    
    for (auto& vod : vodovi) {
        if (vod.id == idVod) {
            vod.stubovi.push_back(stub);
            break;
        }
    }

    dodajUVod(trBrPotrosaca - 1, potrosaci, vodovi);
}

struct Konekcija {
    Stub stub1, stub2;
    float struja;
    float rastojanje;
};

std::vector<Konekcija> skupKonekcija;

void napraviKonekcije(std::vector<Konekcija>& skupKonekcija, const std::vector<Vod>& vodovi, int brStubova, int trVodInd) {
    if (brStubova == 0) return;

    for (int i = trVodInd; i < vodovi.size(); i++) {
        for (int j = vodovi[i].stubovi.size() - 1; j > 0; j--) {
            // Proveri da li konekcija već postoji
            bool postoji = false;
            for (const auto& k : skupKonekcija) {
                if ((k.stub1.id == vodovi[i].stubovi[j].id && k.stub2.id == vodovi[i].stubovi[j - 1].id) ||
                    (k.stub1.id == vodovi[i].stubovi[j - 1].id && k.stub2.id == vodovi[i].stubovi[j].id)) {
                    postoji = true;
                    break;
                }
            }

            if (!postoji) {
                Konekcija knkcija = {vodovi[i].stubovi[j], vodovi[i].stubovi[j - 1], 0, 0};
                if(vodovi[i].stubovi[j].id != vodovi[i].stubovi[j - 1].id) {

                    skupKonekcija.push_back(knkcija);
                    graf[vodovi[i].stubovi[j].id].push_back(vodovi[i].stubovi[j - 1].id);
                }
                
            }
        }
    }
}

std::map<int, std::vector<int>> pronadjiPutanje(const std::vector<Konekcija>& konekcije) {
    std::map<int, std::vector<int>> graf;
    std::map<int, std::vector<int>> putanje;

    // Kreiranje grafa
    for (const auto& k : konekcije) {
        graf[k.stub1.id].push_back(k.stub2.id);
        graf[k.stub2.id].push_back(k.stub1.id);
    }

    // Pronalazak trafo stanice (stub sa najmanjim ID-jem)
    int trafoStub = konekcije[0].stub1.id;
    for (const auto& k : konekcije) {
        if (k.stub1.id < trafoStub) trafoStub = k.stub1.id;
        if (k.stub2.id < trafoStub) trafoStub = k.stub2.id;
    }

    std::cout << "Trafo stanica je stub: " << trafoStub << std::endl;

    // BFS za svaku putanju
    for (const auto& par : graf) {
        int pocetniStub = par.first;
        std::queue<std::vector<int>> red;
        std::map<int, bool> posecen;

        red.push({pocetniStub});
        posecen[pocetniStub] = true;

        while (!red.empty()) {
            std::vector<int> trenutniPut = red.front();
            red.pop();

            int trenutniStub = trenutniPut.back();
            if (trenutniStub == trafoStub) {
                putanje[pocetniStub] = trenutniPut;
                break;
            }

            for (int sused : graf[trenutniStub]) {
                if (!posecen[sused]) {
                    std::vector<int> noviPut = trenutniPut;
                    noviPut.push_back(sused);
                    red.push(noviPut);
                    posecen[sused] = true;
                }
            }
        }
    }

    return putanje;
}

float racunajStruju(int stub, int prethodni) {
    float ukupnaStruja = strujeStubova[stub];

    for (int sused : graf[stub]) {
        if (sused == prethodni) continue; // Izbegavamo povratak

        float strujaSuseda = racunajStruju(sused, stub);
        ukupnaStruja += strujaSuseda;

        // Ažuriranje struje u konekciji
        for (auto& konekcija : skupKonekcija) {
            if ((konekcija.stub1.id == stub && konekcija.stub2.id == sused) ||
                (konekcija.stub1.id == sused && konekcija.stub2.id == stub)) {
                konekcija.struja = strujaSuseda; // Ažuriraj struju samo jednom
                break;
            }
        }
    }

    return ukupnaStruja;
}

int main() {
    std::ifstream mrezaFajl("stubovi.txt");
    if (!mrezaFajl) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    std::string linijaFajla;
    int brPotrosaca = 0;
    std::set<Stub> stubovi;
    std::vector<Potrosac> potrosaci;
    std::vector<Vod> vodovi;

    int stariId = -1;

    while (getline(mrezaFajl, linijaFajla)) {
        std::istringstream iss(linijaFajla);
        Potrosac noviPotrosac;
        iss >> noviPotrosac.ime >> noviPotrosac.vod.id >> noviPotrosac.stub.id >> noviPotrosac.struja;

        strujeStubova[noviPotrosac.stub.id] += noviPotrosac.struja;

        potrosaci.push_back(noviPotrosac);
        brPotrosaca++;

        if (noviPotrosac.vod.id != stariId) {
            Vod noviVod;
            noviVod.id = noviPotrosac.vod.id;
            vodovi.push_back(noviVod);
        }

        stubovi.insert(noviPotrosac.stub);
        stariId = noviPotrosac.vod.id;
    }

    dodajUVod(brPotrosaca - 1, potrosaci, vodovi);

    for (const auto& vod : vodovi) {
        std::cout << "Vod ID: " << vod.id << " - Stubovi: ";
        for (const auto& stub : vod.stubovi) {
            std::cout << stub.id << " ";
        }
        std::cout << std::endl;
    }

    int brStubova = stubovi.size();
    napraviKonekcije(skupKonekcija, vodovi, brStubova, 0);

    std::map<int, std::vector<int>> putanje = pronadjiPutanje(skupKonekcija);
    for (const auto& p : putanje) {
        for (const auto& v : p.second) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }

    int trafoStub = stubovi.begin()->id;
    racunajStruju(trafoStub, -1);

    std::cout << "Struja kroz svaku konekciju:" << std::endl;
    for (int i = 0; i < skupKonekcija.size(); i++) {
        if(skupKonekcija[i].stub1.id == skupKonekcija[i].stub2.id) {
            skupKonekcija[i].struja -= skupKonekcija[i].stub1.ulaznaStruja;
        }
    }

    for(int i = 0; i < skupKonekcija.size(); i++) {
        std::cout << "Skup konekcija izmedju " << skupKonekcija[i].stub1.id <<
                  " i " << skupKonekcija[i].stub2.id << " nosi struju: " << skupKonekcija[i].struja << std::endl; 
    }

    float sum = 0;
    for (const auto& konekcija : skupKonekcija) {
        sum += konekcija.struja * konekcija.struja;
    }

    std::cout << "Ukupna snaga: " << sum << " W" << std::endl;

    return 0;
}