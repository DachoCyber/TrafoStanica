#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <tuple>
#include <map>
#include <queue>

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

void napraviKonekcije(std::vector<Konekcija>& skupKonekcija, const std::vector<Vod>& vodovi, int brStubova, int trVodInd) {
    
    if (brStubova == 0) return;

    for (int i = trVodInd; i < vodovi.size(); i++) {
        for (int j = vodovi[i].stubovi.size() - 1; j > 0; j--) {
            Konekcija knkcija = {vodovi[i].stubovi[j], vodovi[i].stubovi[j - 1], 100, 0};
            skupKonekcija.push_back(knkcija);
        }
    }
}

void izracunajStruje(std::vector<Konekcija>& konekcije, int pocetniStub, double pocetnaStruja) {
    std::map<Stub, std::vector<Stub>> graf;
    std::map<Stub, double> struje;


    // Kreiranje grafa
    for (const auto& konekcija : konekcije) {
        graf[konekcija.stub1].push_back(konekcija.stub2);
    }

    std::cout << pocetnaStruja << std::endl;

    // Postavljanje početne struje
    struje[konekcije[0].stub1] = pocetnaStruja;

    // Red za BFS
    std::queue<Stub> red;
    red.push(konekcije[0].stub1); // Početni stub

    // BFS za raspodelu struje
    while (!red.empty()) {
        Stub trenutniStub = red.front();
        red.pop();

        // Broj odlaznih grana
        int brojOdlaznihStruja = graf[trenutniStub].size();

        if (brojOdlaznihStruja == 0) continue; // Ako nema odlaznih grana, preskoči

        // Struja po grani
        double strujaPoGrani = struje[trenutniStub] / brojOdlaznihStruja;

        // Prolazak kroz sve odlazne grane
        for (const auto& sused : graf[trenutniStub]) {
            struje[sused] = strujaPoGrani;
            red.push(sused);

            // Ažuriranje struje u konekcijama
            for (auto& konekcija : konekcije) {
                if (konekcija.stub1.id == trenutniStub.id && konekcija.stub2.id == sused.id) {
                    konekcija.struja = strujaPoGrani;
                }
            }
        }
    }
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
        iss >> noviPotrosac.ime >> noviPotrosac.vod.id >> noviPotrosac.stub.id;

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

    std::vector<Konekcija> skupKonekcija;
    Stub trStub = vodovi[0].stubovi[0];

    int brStubova = stubovi.size();
    napraviKonekcije(skupKonekcija, vodovi, brStubova, 0);

    izracunajStruje(skupKonekcija, vodovi[0].stubovi[0].id, 100);

    float sum = 0;
    for(int i = 0; i < skupKonekcija.size(); i++) {
        std::cout << skupKonekcija[i].stub1.id << " " << skupKonekcija[i].stub2.id << std::endl;
        std::cout << skupKonekcija[i].struja << std::endl;
        sum += skupKonekcija[i].struja * skupKonekcija[i].struja;
    }

    std::cout << sum << std::endl;

    return 0;
}