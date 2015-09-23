#ifndef DECLARATION
#define DECLARATION
#define EPSILON  0.00001;

#include "inclusions.h"

	int N_STAZIONI ;
	int N_TIME_STEPS ;
	int N_MACCHINE ;
	int CARICA_MAX ;
	int N_RELOCATORS ;
	int B ;
	int CPLEX_TIME;
	int N_AC;
		

enum tipo{WAIT=0,TRAVEL=1,RELOCATION=2,TRANSFER=3};

using namespace boost;

//Struct per le proprietà del grafo

struct station_name_t {
          typedef vertex_property_tag kind;
        };
struct time_step_t {
          typedef vertex_property_tag kind;
        };
struct capacity_t {
          typedef vertex_property_tag kind;
        };

struct tipo_arco_t {
typedef edge_property_tag kind;
};




/* Proprietà vertici
 * Ad ogni vertice sono associati 4 interi, che memorizzano:
 * -Nome della stazione {1..N_STAZIONI}
 * -Time step {1..N_TIME_STEPS}
 * -Numero di parcheggi
 * -Capacità
 */
typedef property<time_step_t, int> Time_prop;
typedef property<station_name_t, int, Time_prop> Time_Name_prop;
typedef property<capacity_t, int, Time_Name_prop> VertexP;

/* Proprietà edge
 *	Ad ogni vertice sono associati 2 double, che memorizzano:
 * -Profitto (weight)
 * -Carica (weight2)
 * Inoltre è presente un booleano che indica se è di tipo relocation (magari sostituire con int??)
 */
typedef property<edge_weight_t, double,
        property<edge_weight2_t,double, property <tipo_arco_t, tipo> > > EdgeP ;


typedef adjacency_list<multisetS, vecS, directedS,VertexP, EdgeP> graph;

property_map<adjacency_list<multisetS, vecS, directedS,VertexP, EdgeP>, edge_weight_t>::type profitto;
property_map<adjacency_list<multisetS, vecS, directedS,VertexP, EdgeP>, edge_weight2_t>::type carica;
property_map<adjacency_list<multisetS, vecS, directedS,VertexP, EdgeP>,tipo_arco_t>::type tipo_arco;

//std::map < boost::graph_traits<adjacency_list<> >::edge_descriptor, std::pair<int,int> > propr;

/* Funzione di inserimento archi relocation (che sono anche di transfer se si muovono in macchina)
 * La seguente funzione inserisce nel grafo g tutti i vertici che partono dalla stazione partenza
 * e arrivano alla stazione destionazione, impiegando un numero di time steps pari a durata.
 * Per ciascun arco, vengono inserite carica e profitto.
 * La funzione necessita infine della mappa dei vertici, che permette di trovare l'indice del vertice
 * corrispondente ad una coppia time_step/nome_stazione.
 * Creare anche doppione di relocation??
 * NB Gli archi di ricarica sono costruiti nel main
 *
 * NBBBBBBBB Se sono anche di transfer, la carica non conta!!
 */
void a_r (int partenza, int destinazione,	int durata, double prof, double car,
adjacency_list<multisetS, vecS, directedS,VertexP, EdgeP>& g,
std::map<std::pair<int,int>, int > vertici) {

std::pair<int,int> id_v;
std::pair<boost::graph_traits<adjacency_list<> >::edge_descriptor,bool> e, e1;
boost::graph_traits<adjacency_list<> >::edge_descriptor e_d;
int edge_source, edge_target;

	for (int t=1; t<= (N_TIME_STEPS-durata); t++) {  //NB nella map indici partono da 1
   		//std::cout<< t << std::endl;
   		id_v=std::make_pair(partenza,t);
   		edge_source =vertici[id_v];
   		//std::cout<< "source " << edge_source << std::endl;
   		id_v=std::make_pair( destinazione, (t+durata) );
   		edge_target =vertici[id_v];
   		//std::cout<< "target time " << (t+durata) << std::endl;

   		e=add_edge(edge_source, edge_target,g);
   		e_d=e.first;
   		profitto[e_d]=prof;
   		carica[e_d]=car;
   		tipo_arco[e_d]=RELOCATION;

		e1=add_edge(edge_source,edge_target,g);
   		e_d=e1.first;
   		profitto[e_d]=-1;
   		carica[e_d]=0;
   		tipo_arco[e_d]=TRANSFER;

		   		//std::cout<< "Aggiunto arco " << e_d << " con profitto "<<profitto[e_d]
			//	 <<" e carica "<< carica [e_d] << std::endl;

   	   }
}


/* Funzione di inserimento archi cliente
 * Il valore opzionale domanda indica quanti clienti richiedono l'arco.
 * NB Attenzione a partenza/destinazione, se vanno oltre il numero di stazioni c'è il rischio di avere
 * archi che vanno indietro nel tempo
 */

void a_c	(int partenza, int destinazione,int inizio, int durata, double prof, double car,
			adjacency_list<multisetS, vecS, directedS,VertexP, EdgeP>& g,
			std::map<std::pair<int,int>, int > vertici, int domanda =1) {

	std::pair<int,int> id_v;
	std::pair<boost::graph_traits<adjacency_list<> >::edge_descriptor,bool> e;
	boost::graph_traits<adjacency_list<> >::edge_descriptor e_d;

		id_v=std::make_pair(partenza,inizio);
   	   	int edge_source =vertici[id_v];
   	   	id_v=std::make_pair( destinazione, (inizio+durata) );
   	   	int edge_target =vertici[id_v];

   	   	e=add_edge(edge_source, edge_target,g);
   	   	e_d=e.first;
   	   	profitto[e_d]=prof;
   	   	carica[e_d]=car;
   	   	tipo_arco[e_d]=TRAVEL;
		//std::cout<< "Aggiunto arco cliente " << e_d << " con profitto "<<profitto[e_d]
			//	 <<" e carica "<< carica [e_d] << std::endl;

}

#endif