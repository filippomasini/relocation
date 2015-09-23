/*    GRAFO DI PROVA    19/03/2015
 *
 *     ATTENZIONE:     struttura non adatta per rimozione vertici,risultano problemi con i puntatori
 *	29/6 rimossi anche transfer dal primo rolling
 *	1/7 i vettori iniziali solo allocati dinamicamente con new e liberati con delete prima del primo solve (such pro)
 *   	8/7 param single thread; rimossa capacity dalle node prop; nomi nn in commento; rimosso lazy_c;aggiunti clear oltre all'end
 * 	10/7 modificati intervalli di rolling per contare anche istante 0 (da <= && > a < && >=);rimosso ultimo solve inutile
 *	15/7 aggiunti name alle constr usando nn
 * 	7/9 stampo sia travel che relocation
 *	14/9 verso linea 1139 confrontato getvalue con valore salvato da round; nulla.
 *	15/9 aggiunto ultimo argomento per scegliere metodo soluzione (vincoli vecchi --> 0 vs nuovi --> 1)
 */

#include "declarations.h"
	

int main(int argc, char *argv[]) {
	
	srand(time(NULL));
	std::cout<<"PARTITO!!";
	

 
   CPLEX_TIME=28800;

   N_STAZIONI=14;

   N_TIME_STEPS=atoi(argv[1]);
   double durata_ts= 10*60/N_TIME_STEPS;
   std::cout<<"TS DI DURATA: "<<durata_ts<<"\n\n";

   N_MACCHINE=atoi(argv[2]);

   N_RELOCATORS=atoi(argv[3]);

   int AC= atoi (argv[4]);

   int INTERVALLI = atoi (argv[5]);

   B=4;

   int metodo = atoi (argv[6]);

   int stampa=0;

   CARICA_MAX=150;

	int carica_iniziale[40]= {139,150,114,88,1,44,18,45,69,142,98,71,149,140,66,45,41,100,13,67,143,69,16,129,103,139,128,99,4,138,83,76,29,29,114,106,9,121,39,107};		
	int posizione_iniziale_veicolo[40]={5,11,6,3,14,7,10,2,2,2,9,10,7,8,9,2,10,9,12,5,10,14,13,14,14,2,11,5,1,11,6,3,7,1,4,12,10,14,10,5};
	int posizione_iniziale_relocator[2]={7,11};
	int capacita[14]={13,10,10,10,10,10,7,7,6,7,5,6,7,7};

	
	
	
	std::cout<<"\nPOSIZIONE INIZIALE MACCHINE:\n";
	for(unsigned int m=0;m<N_MACCHINE;m++)	{
		std::cout<<posizione_iniziale_veicolo[m]<<'\t';
	}

	std::cout<<"\nPOSIZIONE INIZIALE RELOCATORS:\n";
	for(unsigned int r=0;r<N_RELOCATORS;r++)	{
		std::cout<<posizione_iniziale_relocator[r]<<'\t';
	}

	std::cout<<"\nCAPACITA' STAZIONI:\n";
	for(unsigned int s=0;s<N_STAZIONI;s++)		{
		std::cout<<capacita[s]<<'\t';
	}

	std::cout<<std::endl;

		auto start = std::chrono::high_resolution_clock::now();
	
		//classe del grafo
		typedef adjacency_list<multisetS, vecS, directedS,
                            VertexP, EdgeP> graph;
		graph g; 

     	 //property maps dei vertici
     	 property_map<graph, station_name_t>::type station_name
             = get(station_name_t(), g);
         property_map<graph, time_step_t>::type time_step
             = get(time_step_t(), g);

         //property map edges (dichiarate prima, per essere usate nella funzione)
         profitto= get(edge_weight_t(),g);
         carica= get(edge_weight2_t(),g);
         tipo_arco= get(tipo_arco_t(),g);


    /* Mappe dei vertici
     * La prima mappa memorizza le proprietà di un vertice (nome e time step) come key
     * per trovare l'indice che lo identifica nel grafo. La sua presenza potrebbe rendere supreflue
     * le proprietà dei vertici corrisondenti. NB le stazioni partono da indice 1!
	 * La seconda mappa associa l'indice al descrittore vero e proprio.
     */
    	std::map<std::pair<int,int>, int > map_vertici;
	std::map<int,graph::vertex_descriptor > map_vertici2;

	/* Mappe degli archi
     * Nella prima mappa vengono salvati(per ogni intero che corrisponde ad una posizione nell'array di CPLEX)
     * il descriptor e i vertici di partenza/arrivo
	 * Con la seconda è possibile risalire alla posizione se è noto il descriptor.
     */
    std::map <int, std::pair<graph::edge_descriptor, std::pair <graph::vertex_descriptor,graph::vertex_descriptor> > >map_edges;
	std::map <graph::edge_descriptor,int> map_edges2;
	

    //creo vertici
    std::pair<int,int> id_v;
    int num_vertici=0;
    for (int i=0; i < N_STAZIONI ;i++){
        for (int t=0; t< N_TIME_STEPS ;t++){
            add_vertex(g);

            time_step[num_vertici]=t+1;			//aggiungo 1 per evitare lo 0 nel nome
            station_name[num_vertici]=i+1;			//idem
            id_v =std::make_pair(station_name[num_vertici],time_step[num_vertici]);
            map_vertici[id_v]=num_vertici;			//aggiorno mappa dei vertici
            num_vertici++;							//devo eseguire il for anche per num_vertici=0
        }
    }




	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato creaz vertici: "<<elapsed.count()<<std::endl;
	start = std::chrono::high_resolution_clock::now();


    //creo archi ricarica
    num_vertici=0;
    std::pair<boost::graph_traits<graph>::edge_descriptor,bool> e, e1;
    boost::graph_traits<graph>::edge_descriptor e_d;

        for (int i=0; i< N_STAZIONI ;i++){
                for (int t=0; t< N_TIME_STEPS ;t++){     //non valido per ultimo time step
                    if(t!= N_TIME_STEPS-1)  {
                        e=add_edge(num_vertici,num_vertici+1,g);
                        e_d=e.first;
                        profitto[e_d]=0;
                        carica[e_d]=15;
                        tipo_arco[e_d]=WAIT;
										
						e1=add_edge(num_vertici,num_vertici+1,g);
   						e_d=e1.first;
   						profitto[e_d]=0;
   						carica[e_d]=0;
   						tipo_arco[e_d]=WAIT;

                        ++num_vertici;
                    }
                    else ++num_vertici;						//ma devo comunque incrementare
            }
        }



a_r(1,2,1,-3,-2,g,map_vertici);
a_r(1,3,1,-5,-4,g,map_vertici);
a_r(1,4,1,-5,-4,g,map_vertici);
a_r(1,5,1,-3,-2,g,map_vertici);
a_r(1,6,1,-6,-4,g,map_vertici);
a_r(1,7,1,-2,-1,g,map_vertici);
a_r(1,8,1,-7,-5,g,map_vertici);
a_r(1,9,2,-36,-24,g,map_vertici);
a_r(1,10,1,-2,-2,g,map_vertici);
a_r(1,11,1,-6,-4,g,map_vertici);
a_r(1,12,1,-6,-4,g,map_vertici);
a_r(1,13,1,-5,-3,g,map_vertici);
a_r(1,14,1,-11,-7,g,map_vertici);
a_r(2,1,1,-6,-4,g,map_vertici);
a_r(2,3,1,-6,-4,g,map_vertici);
a_r(2,4,1,-7,-5,g,map_vertici);
a_r(2,5,1,-5,-3,g,map_vertici);
a_r(2,6,1,-8,-6,g,map_vertici);
a_r(2,7,1,-3,-2,g,map_vertici);
a_r(2,8,1,-5,-3,g,map_vertici);
a_r(2,9,2,-36,-24,g,map_vertici);
a_r(2,10,1,-4,-3,g,map_vertici);
a_r(2,11,1,-4,-3,g,map_vertici);
a_r(2,12,1,-9,-6,g,map_vertici);
a_r(2,13,1,-8,-5,g,map_vertici);
a_r(2,14,1,-9,-6,g,map_vertici);
a_r(3,1,1,-6,-4,g,map_vertici);
a_r(3,2,1,-7,-5,g,map_vertici);
a_r(3,4,1,-6,-4,g,map_vertici);
a_r(3,5,1,-4,-3,g,map_vertici);
a_r(3,6,1,-7,-5,g,map_vertici);
a_r(3,7,1,-8,-5,g,map_vertici);
a_r(3,8,1,-7,-5,g,map_vertici);
a_r(3,9,2,-30,-20,g,map_vertici);
a_r(3,10,1,-5,-3,g,map_vertici);
a_r(3,11,1,-8,-6,g,map_vertici);
a_r(3,12,1,-7,-5,g,map_vertici);
a_r(3,13,1,-6,-4,g,map_vertici);
a_r(3,14,1,-11,-7,g,map_vertici);
a_r(4,1,1,-5,-3,g,map_vertici);
a_r(4,2,1,-7,-5,g,map_vertici);
a_r(4,3,1,-4,-3,g,map_vertici);
a_r(4,5,1,-6,-4,g,map_vertici);
a_r(4,6,1,-2,-2,g,map_vertici);
a_r(4,7,1,-6,-4,g,map_vertici);
a_r(4,8,1,-11,-7,g,map_vertici);
a_r(4,9,2,-33,-22,g,map_vertici);
a_r(4,10,1,-5,-3,g,map_vertici);
a_r(4,11,1,-9,-6,g,map_vertici);
a_r(4,12,1,-3,-2,g,map_vertici);
a_r(4,13,1,-1,-1,g,map_vertici);
a_r(4,14,1,-14,-10,g,map_vertici);
a_r(5,1,1,-3,-2,g,map_vertici);
a_r(5,2,1,-5,-3,g,map_vertici);
a_r(5,3,1,-5,-3,g,map_vertici);
a_r(5,4,1,-4,-3,g,map_vertici);
a_r(5,6,1,-5,-3,g,map_vertici);
a_r(5,7,1,-3,-2,g,map_vertici);
a_r(5,8,1,-7,-5,g,map_vertici);
a_r(5,9,2,-31,-21,g,map_vertici);
a_r(5,10,1,-2,-2,g,map_vertici);
a_r(5,11,1,-8,-6,g,map_vertici);
a_r(5,12,1,-5,-3,g,map_vertici);
a_r(5,13,1,-3,-2,g,map_vertici);
a_r(5,14,1,-11,-7,g,map_vertici);
a_r(6,1,1,-6,-4,g,map_vertici);
a_r(6,2,1,-9,-6,g,map_vertici);
a_r(6,3,1,-7,-5,g,map_vertici);
a_r(6,4,1,-2,-1,g,map_vertici);
a_r(6,5,1,-9,-6,g,map_vertici);
a_r(6,7,1,-13,-9,g,map_vertici);
a_r(6,8,1,-12,-8,g,map_vertici);
a_r(6,9,2,-33,-22,g,map_vertici);
a_r(6,10,1,-6,-4,g,map_vertici);
a_r(6,11,2,-10,-7,g,map_vertici);
a_r(6,12,1,-3,-2,g,map_vertici);
a_r(6,13,1,-3,-2,g,map_vertici);
a_r(6,14,2,-20,-14,g,map_vertici);
a_r(7,1,1,-2,-2,g,map_vertici);
a_r(7,2,1,-3,-2,g,map_vertici);
a_r(7,3,1,-5,-4,g,map_vertici);
a_r(7,4,1,-6,-4,g,map_vertici);
a_r(7,5,1,-4,-3,g,map_vertici);
a_r(7,6,1,-7,-5,g,map_vertici);
a_r(7,8,1,-7,-5,g,map_vertici);
a_r(7,9,2,-36,-24,g,map_vertici);
a_r(7,10,1,-3,-2,g,map_vertici);
a_r(7,11,1,-6,-4,g,map_vertici);
a_r(7,12,1,-7,-5,g,map_vertici);
a_r(7,13,1,-6,-4,g,map_vertici);
a_r(7,14,1,-11,-7,g,map_vertici);
a_r(8,1,1,-6,-4,g,map_vertici);
a_r(8,2,1,-5,-4,g,map_vertici);
a_r(8,3,1,-9,-6,g,map_vertici);
a_r(8,4,1,-11,-8,g,map_vertici);
a_r(8,5,1,-7,-5,g,map_vertici);
a_r(8,6,1,-12,-8,g,map_vertici);
a_r(8,7,1,-5,-4,g,map_vertici);
a_r(8,9,2,-34,-23,g,map_vertici);
a_r(8,10,1,-6,-4,g,map_vertici);
a_r(8,11,1,-4,-3,g,map_vertici);
a_r(8,12,1,-13,-9,g,map_vertici);
a_r(8,13,1,-10,-7,g,map_vertici);
a_r(8,14,1,-5,-4,g,map_vertici);
a_r(9,1,2,-32,-22,g,map_vertici);
a_r(9,2,2,-35,-24,g,map_vertici);
a_r(9,3,2,-28,-19,g,map_vertici);
a_r(9,4,2,-33,-22,g,map_vertici);
a_r(9,5,2,-30,-20,g,map_vertici);
a_r(9,6,2,-32,-22,g,map_vertici);
a_r(9,7,2,-35,-23,g,map_vertici);
a_r(9,8,2,-34,-23,g,map_vertici);
a_r(9,10,2,-31,-21,g,map_vertici);
a_r(9,11,2,-36,-24,g,map_vertici);
a_r(9,12,2,-33,-22,g,map_vertici);
a_r(9,13,2,-32,-22,g,map_vertici);
a_r(9,14,2,-32,-22,g,map_vertici);
a_r(10,1,1,-1,-1,g,map_vertici);
a_r(10,2,1,-3,-2,g,map_vertici);
a_r(10,3,1,-5,-4,g,map_vertici);
a_r(10,4,1,-5,-4,g,map_vertici);
a_r(10,5,1,-3,-2,g,map_vertici);
a_r(10,6,1,-6,-4,g,map_vertici);
a_r(10,7,1,-2,-1,g,map_vertici);
a_r(10,8,1,-7,-5,g,map_vertici);
a_r(10,9,2,-36,-24,g,map_vertici);
a_r(10,11,1,-6,-4,g,map_vertici);
a_r(10,12,1,-6,-4,g,map_vertici);
a_r(10,13,1,-5,-3,g,map_vertici);
a_r(10,14,1,-11,-7,g,map_vertici);
a_r(11,1,1,-4,-3,g,map_vertici);
a_r(11,2,1,-4,-3,g,map_vertici);
a_r(11,3,1,-7,-5,g,map_vertici);
a_r(11,4,1,-8,-6,g,map_vertici);
a_r(11,5,1,-6,-4,g,map_vertici);
a_r(11,6,2,-9,-6,g,map_vertici);
a_r(11,7,1,-3,-2,g,map_vertici);
a_r(11,8,1,-5,-3,g,map_vertici);
a_r(11,9,2,-36,-24,g,map_vertici);
a_r(11,10,1,-5,-3,g,map_vertici);
a_r(11,12,2,-9,-6,g,map_vertici);
a_r(11,13,1,-8,-6,g,map_vertici);
a_r(11,14,1,-8,-6,g,map_vertici);
a_r(12,1,1,-7,-5,g,map_vertici);
a_r(12,2,1,-8,-6,g,map_vertici);
a_r(12,3,1,-9,-6,g,map_vertici);
a_r(12,4,1,-3,-2,g,map_vertici);
a_r(12,5,1,-7,-5,g,map_vertici);
a_r(12,6,1,-3,-2,g,map_vertici);
a_r(12,7,1,-7,-5,g,map_vertici);
a_r(12,8,1,-12,-8,g,map_vertici);
a_r(12,9,2,-34,-23,g,map_vertici);
a_r(12,10,1,-6,-4,g,map_vertici);
a_r(12,11,2,-12,-8,g,map_vertici);
a_r(12,13,1,-4,-3,g,map_vertici);
a_r(12,14,1,-15,-10,g,map_vertici);
a_r(13,1,1,-5,-3,g,map_vertici);
a_r(13,2,1,-6,-4,g,map_vertici);
a_r(13,3,1,-5,-3,g,map_vertici);
a_r(13,4,1,-2,-1,g,map_vertici);
a_r(13,5,1,-5,-3,g,map_vertici);
a_r(13,6,1,-3,-2,g,map_vertici);
a_r(13,7,1,-5,-4,g,map_vertici);
a_r(13,8,1,-11,-7,g,map_vertici);
a_r(13,9,2,-33,-22,g,map_vertici);
a_r(13,10,1,-4,-3,g,map_vertici);
a_r(13,11,1,-9,-6,g,map_vertici);
a_r(13,12,1,-3,-2,g,map_vertici);
a_r(13,14,1,-14,-10,g,map_vertici);
a_r(14,1,1,-11,-8,g,map_vertici);
a_r(14,2,1,-10,-7,g,map_vertici);
a_r(14,3,1,-15,-10,g,map_vertici);
a_r(14,4,2,-19,-13,g,map_vertici);
a_r(14,5,1,-11,-8,g,map_vertici);
a_r(14,6,2,-19,-13,g,map_vertici);
a_r(14,7,1,-10,-7,g,map_vertici);
a_r(14,8,1,-6,-4,g,map_vertici);
a_r(14,9,2,-30,-20,g,map_vertici);
a_r(14,10,1,-11,-7,g,map_vertici);
a_r(14,11,1,-9,-6,g,map_vertici);
a_r(14,12,2,-20,-13,g,map_vertici);
a_r(14,13,2,-15,-10,g,map_vertici);

a_c(4,11,10,14,57,-33,g,map_vertici);
a_c(11,5,4,13,62,-90,g,map_vertici);
a_c(12,12,0,13,47,-18,g,map_vertici);
a_c(5,5,6,12,56,-75,g,map_vertici);
a_c(4,5,2,11,49,-53,g,map_vertici);
a_c(3,8,9,10,40,-25,g,map_vertici);
a_c(5,5,8,10,43,-50,g,map_vertici);
a_c(12,4,12,10,45,-57,g,map_vertici);
a_c(12,12,12,7,27,-23,g,map_vertici);
a_c(12,5,6,6,27,-32,g,map_vertici);
a_c(8,10,8,6,24,-25,g,map_vertici);
a_c(6,6,2,5,30,-59,g,map_vertici);
a_c(4,4,10,5,32,-68,g,map_vertici);
a_c(4,4,10,5,27,-42,g,map_vertici);
a_c(5,3,0,6,31,-60,g,map_vertici);
a_c(3,3,5,5,21,-18,g,map_vertici);
a_c(4,4,2,5,17,-7,g,map_vertici);
a_c(4,4,10,4,24,-51,g,map_vertici);
a_c(8,8,10,6,25,-18,g,map_vertici);
a_c(6,13,5,4,18,-31,g,map_vertici);
a_c(13,13,4,3,15,-19,g,map_vertici);
a_c(6,5,8,3,21,-51,g,map_vertici);
a_c(6,6,11,3,20,-49,g,map_vertici);
a_c(12,12,1,3,12,-8,g,map_vertici);
a_c(4,4,13,3,12,-16,g,map_vertici);
a_c(3,3,6,3,12,-16,g,map_vertici);
a_c(6,6,8,3,19,-53,g,map_vertici);
a_c(5,5,10,3,10,-9,g,map_vertici);
a_c(2,2,4,2,12,-21,g,map_vertici);
a_c(6,6,13,2,9,-9,g,map_vertici);
a_c(12,5,13,2,7,-5,g,map_vertici);
a_c(7,10,15,2,6,-5,g,map_vertici);
a_c(6,2,8,2,6,-7,g,map_vertici);
a_c(5,3,9,1,5,-5,g,map_vertici);
a_c(13,6,6,1,4,-3,g,map_vertici);
a_c(10,8,6,1,5,-9,g,map_vertici);
a_c(2,12,9,1,5,-7,g,map_vertici);
a_c(5,9,10,1,7,-20,g,map_vertici);
a_c(10,13,5,1,3,-2,g,map_vertici);
a_c(13,10,5,1,2,-2,g,map_vertici);
a_c(12,3,4,1,3,-5,g,map_vertici);
a_c(3,5,7,1,2,-2,g,map_vertici);
a_c(12,10,11,1,3,-4,g,map_vertici);
a_c(6,3,7,1,3,-5,g,map_vertici);
a_c(3,6,11,1,3,-4,g,map_vertici);
a_c(10,7,14,1,2,-2,g,map_vertici);
a_c(3,12,11,1,2,-4,g,map_vertici);
a_c(13,5,15,1,2,-2,g,map_vertici);
a_c(10,13,13,1,2,-3,g,map_vertici);
a_c(10,13,4,1,2,-2,g,map_vertici);
 
a_c(6,3,6,1,3,-4,g,map_vertici);
a_c(4,4,15,23,46,-64,g,map_vertici);
a_c(3,5,24,1,3,-3,g,map_vertici);
a_c(3,4,18,1,3,-4,g,map_vertici);
a_c(3,6,20,1,2,-4,g,map_vertici); 
a_c(5,5,6,10,21,-34,g,map_vertici);
a_c(4,4,22,12,29,-55,g,map_vertici);
a_c(5,4,12,3,5,-5,g,map_vertici);
a_c(1,5,5,2,4,-8,g,map_vertici);
a_c(3,3,17,9,18,-23,g,map_vertici);
a_c(4,4,22,9,20,-36,g,map_vertici);
a_c(5,6,14,10,24,-47,g,map_vertici);
a_c(2,6,24,2,4,-6,g,map_vertici);
a_c(2,3,23,14,30,-51,g,map_vertici);
a_c(3,6,19,1,2,-4,g,map_vertici);


a_c(5,5,10,7,19,-44,g,map_vertici);
a_c(4,4,23,10,24,-45,g,map_vertici);
a_c(5,5,28,3,5,-7,g,map_vertici);
a_c(5,5,-14,2,5,-13,g,map_vertici);
a_c(2,5,24,2,3,-3,g,map_vertici);
a_c(4,4,22,12,29,-53,g,map_vertici);
a_c(4,4,21,18,42,-76,g,map_vertici);
a_c(6,6,16,6,11,-16,g,map_vertici);
a_c(4,4,17,5,12,-27,g,map_vertici);
a_c(2,5,5,10,20,-26,g,map_vertici);
a_c(2,2,14,9,16,-13,g,map_vertici);
a_c(4,4,18,4,12,-26,g,map_vertici);
a_c(6,3,22,1,2,-4,g,map_vertici);
a_c(3,6,33,1,2,-4,g,map_vertici);
a_c(5,5,13,7,13,-13,g,map_vertici);


a_c(3,6,7,1,3,-6,g,map_vertici);
a_c(2,2,27,11,18,-7,g,map_vertici);
a_c(6,3,17,1,2,-3,g,map_vertici);
a_c(4,5,12,8,18,-29,g,map_vertici);
a_c(3,2,22,1,3,-5,g,map_vertici);
a_c(2,3,9,1,3,-4,g,map_vertici);
a_c(4,4,13,14,33,-58,g,map_vertici);
a_c(4,4,8,5,15,-39,g,map_vertici);
a_c(2,2,8,9,22,-46,g,map_vertici);
a_c(3,3,16,3,7,-13,g,map_vertici);
a_c(2,3,0,2,3,-4,g,map_vertici);
a_c(3,3,20,4,7,-7,g,map_vertici);
a_c(2,5,6,15,32,-47,g,map_vertici);
a_c(5,3,14,11,28,-64,g,map_vertici);
a_c(4,3,23,1,2,-3,g,map_vertici);

a_c(3,2,22,1,3,-5,g,map_vertici);
a_c(4,4,22,13,35,-79,g,map_vertici);
a_c(6,6,8,23,47,-66,g,map_vertici);
a_c(3,6,32,1,3,-4,g,map_vertici);
a_c(2,2,12,19,39,-52,g,map_vertici);
a_c(4,4,22,5,10,-9,g,map_vertici);
a_c(5,5,17,8,21,-46,g,map_vertici);
a_c(3,3,24,13,27,-42,g,map_vertici);
a_c(3,2,17,7,14,-22,g,map_vertici);
a_c(4,4,20,13,27,-39,g,map_vertici);
a_c(5,3,23,1,2,-1,g,map_vertici);
a_c(2,2,8,9,22,-46,g,map_vertici);
a_c(5,5,18,10,24,-43,g,map_vertici);
a_c(4,4,8,5,18,-55,g,map_vertici);
a_c(3,6,29,9,19,-29,g,map_vertici);
a_c(3,3,15,7,14,-19,g,map_vertici);		//asfadfdfafadfdafadfadf
a_c(3,3,21,1,3,-6,g,map_vertici);
a_c(2,2,25,9,19,-27,g,map_vertici);
a_c(2,2,24,14,26,-25,g,map_vertici);
a_c(6,2,36,2,3,-5,g,map_vertici);
a_c(6,2,32,2,4,-6,g,map_vertici);
a_c(6,6,7,12,30,-62,g,map_vertici);
a_c(3,6,9,4,11,-22,g,map_vertici);
a_c(4,3,32,2,3,-3,g,map_vertici);
a_c(5,2,19,3,4,-4,g,map_vertici);
a_c(6,2,32,4,11,-29,g,map_vertici);
a_c(2,2,28,4,13,-38,g,map_vertici);
a_c(6,6,22,16,37,-66,g,map_vertici);
a_c(6,6,24,6,13,-22,g,map_vertici);
a_c(2,3,21,1,3,-4,g,map_vertici);
a_c(6,4,23,7,16,-28,g,map_vertici);
a_c(3,4,18,1,3,-4,g,map_vertici);
a_c(6,6,17,13,32,-65,g,map_vertici);
a_c(5,3,-4,1,2,-2,g,map_vertici);
a_c(3,6,29,1,2,-4,g,map_vertici);
a_c(6,2,17,11,19,-11,g,map_vertici);
a_c(6,6,10,8,23,-55,g,map_vertici);
a_c(4,3,34,1,2,-3,g,map_vertici);
a_c(6,6,14,5,9,-6,g,map_vertici);
a_c(4,4,5,7,14,-24,g,map_vertici);
a_c(3,2,19,2,4,-5,g,map_vertici);
a_c(4,4,3,10,26,-56,g,map_vertici);
a_c(3,3,15,5,12,-25,g,map_vertici);
a_c(3,4,18,1,3,-4,g,map_vertici);
a_c(6,5,12,10,22,-40,g,map_vertici);
a_c(2,2,27,6,12,-18,g,map_vertici);
a_c(3,5,16,3,5,-4,g,map_vertici);
a_c(6,4,19,10,19,-23,g,map_vertici);
a_c(4,3,17,4,11,-28,g,map_vertici);
a_c(6,6,7,26,52,-68,g,map_vertici);

	



//CPLEX
IloEnv env;	

	
try{
	IloModel model(env);
	 IloCplex cplex(env); 

	/* CREAZIONE VARIABILI ARCHI 
	* Vengono considerati TUTTI gli archi presenti nel grafo
	* Si crea un array di array. Le variabili corrispondenti a ciascun arco saranno ripetute
	* per ciascuna macchina/relocator
	*/


    IloArray <IloIntVarArray> v_cliente(env);
    IloArray <IloIntVarArray> v_relocator(env);
    int edge_id=0;
	graph_traits<graph>::edge_iterator ei,ei_end;
	for(unsigned int m=0;m<N_MACCHINE;m++)
		v_cliente.add(IloIntVarArray(env));
	for(unsigned int r=0;r<N_RELOCATORS;r++)
		v_relocator.add(IloIntVarArray(env));
    boost::tie(ei, ei_end) = edges(g);														
    for( ; ei != ei_end; ++ei){
		map_edges2[*ei]=edge_id;
    	map_edges[edge_id]=std::make_pair(*ei,std::make_pair(source(*ei,g),target(*ei,g)));
		if(tipo_arco[*ei]!=TRANSFER){
			for(unsigned int m=0;m<N_MACCHINE;m++)			//creo variabili anche per archi che non servono, così per ogni array di variabili
				v_cliente[m].add(IloIntVar(env,0,1));		//la posizione x corrisponde all'arco x della mappa; altrimenti dovrei fare altre mappe
		}
		if(tipo_arco[*ei]==TRANSFER){

			for(unsigned int m=0;m<N_MACCHINE;m++)			
				v_cliente[m].add(IloIntVar(env,0,0));
		}
		if (tipo_arco[*ei]==TRANSFER || tipo_arco[*ei]==RELOCATION|| tipo_arco[*ei]==WAIT){

			for(unsigned int r=0;r<N_RELOCATORS;r++) 
				v_relocator[r].add(IloIntVar(env,0,1));
		}
		if (tipo_arco[*ei]!=TRANSFER && tipo_arco[*ei]!=RELOCATION && tipo_arco[*ei]!=WAIT){

			for(unsigned int r=0;r<N_RELOCATORS;r++) 
				v_relocator[r].add(IloIntVar(env,0,0));
		}
		edge_id++;
	}
	for(unsigned int r=0;r<N_RELOCATORS;r++){
		model.add(v_relocator[r]);
	}
	for(unsigned int m=0;m<N_MACCHINE;m++){
		model.add(v_cliente[m]);

	}	


	// CREAZIONE VARIABILI CARICA
	IloArray <IloIntVarArray> v_carica(env);
	std::stringstream nn;
	for(unsigned int m=0;m<N_MACCHINE;m++){
		v_carica.add(IloIntVarArray(env));
		for (unsigned int j=0;j<=N_TIME_STEPS;j++){				
			v_carica[m].add(IloIntVar(env,0,CARICA_MAX));
			//if(stampa==1)
			//	<< "Carica m "<<m<<" tempo "<<j+1;
			//v_carica[m][j].setName( nn.str().c_str() );		
			//nn.clear();
			//nn.str(std::string());
		}
		model.add(v_carica[m]);
	}


    //Fine creazione variabili



	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato creaz variabili: "<<elapsed.count()<<std::endl;
	start = std::chrono::high_resolution_clock::now();






	graph::edge_descriptor edge;

	/* CREAZIONE FO
	* Si utilizza un'espessione nella forma
	*	v_cliente*profitto_arco
	*/

    IloExpr e_profitto(env);							
	
    for(unsigned int i=0; i<map_edges.size();i++){				
    	edge=map_edges[i].first;	
		for(unsigned int r=0;r<N_RELOCATORS;r++){
			if (tipo_arco[edge]==TRANSFER || tipo_arco[edge]==RELOCATION|| tipo_arco[edge]==WAIT){
				nn	<< "(" << map_edges[i].second.first <<"-"<< map_edges[i].second.second 
					<<";r"<<r<<" tipo "<<tipo_arco[edge]<< ")";				
				v_relocator[r][i].setName( nn.str().c_str() );		
				nn.clear();
				nn.str(std::string());							
				if(tipo_arco[edge]==TRANSFER){					//costo (profitto) solo per transfer
					e_profitto+=profitto[edge]*v_relocator[r][i];					
				}
			}
		}



		for(unsigned int m=0;m<N_MACCHINE;m++){	
				//nn	<< "(" << map_edges[i].second.first <<"-"<< map_edges[i].second.second 
				//	<<";m"<<m<<" tipo "<<tipo_arco[edge]<< ")";
				//v_cliente[m][i].setName( nn.str().c_str() );				//assegno al vertice un nome source-target
				//nn.clear();
				//nn.str(std::string());										//svuoto stringstream
				if(tipo_arco[edge]==TRAVEL||tipo_arco[edge]==RELOCATION)	//per i wait non serve
					e_profitto+=profitto[edge]*v_cliente[m][i];				//costruisco l'espressione per la fo			
		}
    }	
	model.add(IloMaximize(env,e_profitto));
	if(stampa==1)
		std::cout << "Funzione Obiettivo:"<<'\n'<< "MAX "<<e_profitto << std::endl;
	std::cout<<std::endl;
	e_profitto.end();
	//fine FO


	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato creaz FO: "<<elapsed.count()<<std::endl;
	start = std::chrono::high_resolution_clock::now();









	/* CREAZIONE VINCOLI*/
    int current_vertex, current_edge;
    
			
	IloExprArray e_domanda(env);
	IloExprArray e_relocator_for_car(env);
	IloExprArray e_car_for_relocator(env);
	IloExprArray e_capacita(env);

	IloExprArray e_vincolo_partenza_veicolo(env);		//espressioni/vincoli per veicoli
	IloExprArray e_vincolo_posiz_iniziale_veicolo(env);
	IloArray<IloExprArray > e_cons_flusso_veicolo(env);
	IloRangeArray cons_flusso_veicolo(env);
	 
	IloExprArray e_vincolo_partenza_relocator(env);		//espressioni/vincoli per relocators
	IloExprArray e_vincolo_posiz_iniziale_relocator(env);
	IloArray<IloExprArray > e_cons_flusso_relocator(env);
	IloRangeArray cons_flusso_relocator(env);

	for(unsigned int m=0;m<N_MACCHINE;m++){				//Aggiungo agli array elementi per tutte le macchine
		e_vincolo_partenza_veicolo.add(IloExpr(env));	
		e_cons_flusso_veicolo.add(IloExprArray(env));
		e_vincolo_posiz_iniziale_veicolo.add(IloExpr(env));
		cons_flusso_veicolo.add(IloRangeArray(env));	//Creo m array di conservazione flusso; per ciascun m-esimo array aggiungo un array per ogni edge(sotto)
	}

	for(unsigned int r=0;r<N_RELOCATORS;r++){			//Aggiungo agli array elementi per tutti i relocator
		e_vincolo_partenza_relocator.add(IloExpr(env));
		e_cons_flusso_relocator.add(IloExprArray(env));
		e_vincolo_posiz_iniziale_relocator.add(IloExpr(env));
		cons_flusso_relocator.add(IloRangeArray(env));
	}
			
	//Vincolo domanda separato. L'espressione va aggiornata per ogni edge ma non ripetuta per ogni stazione i,j
	std::cout	<<"Vincolo domanda:"<<'\n';
	boost::tie(ei, ei_end) = edges(g);														
    for( ; ei != ei_end; ++ei)	{							
		current_edge=map_edges2[*ei];
		e_domanda.add(IloExpr(env));
				
		if (tipo_arco[*ei]==TRAVEL){
			for(unsigned int m=0;m<N_MACCHINE;m++)
				e_domanda[current_edge]+=v_cliente[m][current_edge];		//ogni arco di domanda percorso al massimo da una macchina
			model.add(IloRange(env,0,e_domanda[current_edge],1));			//si suppone domanda=1;altrimenti serve altra prop map
			if(stampa==1)
				std::cout	<<"Arco "<<current_edge <<" : "
							<<IloRange(env,0,e_domanda[current_edge],1)<<"\n";
		}
	}

	e_domanda.clear();
	e_domanda.end();
	std::cout<<std::endl;


	//Vincolo capacità stazioni
	std::cout	<<"Vincolo capacità:"<<'\n';
	boost::tie(ei, ei_end) = edges(g);														
    for( ; ei != ei_end; ++ei)	{							
		current_edge=map_edges2[*ei];
		e_capacita.add(IloExpr(env));	//qui altrimenti non prendo tutti possibili edge e non associo a variabile giusta!
		if (tipo_arco[*ei]==WAIT){
			int current_station = station_name[map_edges[current_edge].second.first];

			for(unsigned int m=0;m<N_MACCHINE;m++)
				e_capacita[current_edge]+=v_cliente[m][current_edge];		
			model.add(IloRange(env,0,e_capacita[current_edge],capacita[current_station-1]));		//il vettore inizia da 0, nella mappa invece inizia da  1
			if(stampa==1)
				std::cout	<<"Arco "<<current_edge <<" : "
							<<IloRange(env,0,e_capacita[current_edge],capacita[current_station-1])<<"\n";
		}
	}

	e_capacita.clear();
	e_capacita.end();
	std::cout<<std::endl;




	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato creaz vincolo domanda e array: "<<elapsed.count()<<std::endl;
	start = std::chrono::high_resolution_clock::now();




	// Vincolo carica 
	std::cout	<<"Vincolo carica:" <<'\n';
	int current_edge_target_time, current_edge_source_time, tau;

	//Nuovo metodo
	if (metodo==1){
		IloArray <IloExprArray>  e_carica(env);
		for(unsigned int m=0;m<N_MACCHINE;m++){				//Aggiungo agli array elementi per tutte le macchine
		e_carica.add(IloExprArray(env));
		for(unsigned int j=0;j<=N_TIME_STEPS;j++)
			e_carica[m].add(IloExpr(env));
			
		}

		for(unsigned int m=0;m<N_MACCHINE;m++){															//Carica per ogni macchina
			model.add(IloRange(env,carica_iniziale[m],v_carica[m][0],carica_iniziale[m])); 
				std::cout	<<"Macch "<<m<<" al tempo 1: "
							<<IloRange(env,carica_iniziale[m],v_carica[m][0],carica_iniziale[m])<<"\n";
			for(unsigned int current_time=1;current_time<N_TIME_STEPS;current_time++){
			
					boost::tie(ei, ei_end) = edges(g);
					for( ; ei != ei_end; ++ei)	{					
						if(tipo_arco[*ei]!=TRANSFER){
							current_edge=map_edges2[*ei];
							current_edge_target_time=time_step[map_edges[current_edge].second.second];		//Time step della destinazione dell'edge
						
							if(current_edge_target_time<=current_time)
								e_carica[m][current_time]+=carica[*ei]*v_cliente[m][current_edge];			//per ogni edge si aggiunge all'espressione la carica	
							}
						}
				
			
				
						model.add(IloRange(env,0,e_carica[m][current_time]+carica_iniziale[m],CARICA_MAX)); 
				
			}
		}
	}




	// Vecchio metodo
	if (metodo ==0){

		IloArray <IloArray <IloExprArray> > e_carica(env);
		for(unsigned int m=0;m<N_MACCHINE;m++){				
			e_carica.add(IloArray <IloExprArray > (env));
			for(unsigned int j=0;j<=N_TIME_STEPS;j++){
				e_carica[m].add(IloExprArray(env));
				for(unsigned int j1=0;j1<=j;j1++)
					e_carica[m][j].add(IloExpr(env));
			}
		}


		for(unsigned int m=0;m<N_MACCHINE;m++){															//Carica per ogni macchina
		model.add(IloRange(env,carica_iniziale[m],v_carica[m][0],carica_iniziale[m])); 
			std::cout	<<"Macch "<<m<<" al tempo 1: "
						<<IloRange(env,carica_iniziale[m],v_carica[m][0],carica_iniziale[m])<<"\n";
		for(unsigned int current_time=1;current_time<N_TIME_STEPS;current_time++){
			tau=0;
			do{
				boost::tie(ei, ei_end) = edges(g);
				for( ; ei != ei_end; ++ei)	{					
					if(tipo_arco[*ei]!=TRANSFER){
						current_edge=map_edges2[*ei];
						//std::cout<< map_edges2[*ei]<<" = "<<" targ ";
						current_edge_target_time=time_step[map_edges[current_edge].second.second];		//Time step della destinazione dell'edge
						current_edge_source_time=time_step[map_edges[current_edge].second.first];
						
						if(current_edge_target_time<=current_time+1 && current_edge_source_time>=tau+1){
							//std::cout<< " sou "<<current_edge_source_time<<" targ " << current_edge_target_time;
							//std::cout<< " t "<<current_time<<" tau " << tau;
							e_carica[m][current_time][tau]+=carica[*ei]*v_cliente[m][current_edge];			//per ogni edge si aggiunge all'espressione la carica	
						}
					}
				}
				if(tau==0){
					nn	<< "car m " << m <<" time  "<<current_time<<" tau "<< tau;
					model.add(IloRange(env,0,e_carica[m][current_time][tau]+v_carica[m][tau]-v_carica[m][current_time],IloInfinity,nn.str().c_str()));
					nn.clear();
					nn.str(std::string());
				}
				else{
					model.add(IloRange(env,0,e_carica[m][current_time][tau]+v_carica[m][tau]-v_carica[m][current_time],IloInfinity)); 
				}
				//if(stampa==1)
				//	std::cout	<<"Macch "<<m<<" al tempo "<<current_time+1 <<" partendo da tau "<< tau+1<<" : "
				//				<<IloRange(env,0,e_carica[m][current_time][tau]+v_carica[m][tau]-v_carica[m][current_time],IloInfinity)<<"\n";
				tau++;
			}while(tau<current_time);
		}
	}


	}


/*
	for(unsigned int m=0;m<N_MACCHINE;m++){															//Carica per ogni macchina

	e_carica[m].clear();
	e_carica[m].end();
}
	
	
	e_carica.clear();
	e_carica.end();
	std::cout<<std::endl;
*/


	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato creaz vincoli carica: "<<elapsed.count()<<std::endl;
	start = std::chrono::high_resolution_clock::now();




	//Vincoli: conservazione flusso + nodo origine per veicoli
	std::cout	<<"Vincolo flusso veicoli: "<<'\n';
    for (unsigned int i=0; i<N_STAZIONI;i++){
    	for (unsigned int j=0; j<N_TIME_STEPS;j++){
			for(unsigned int m=0;m<N_MACCHINE;m++)
				e_cons_flusso_veicolo[m].add(IloExpr(env));
    			current_vertex=map_vertici[std::make_pair(i+1,j+1)];										
    			boost::tie(ei, ei_end) = edges(g);														
    			    for( ; ei != ei_end; ++ei)	{							
						current_edge=map_edges2[*ei];
						if(tipo_arco[*ei]!=TRANSFER){
    			    	    if(source(*ei,g)==current_vertex){									
								if(j==0)	{										//Per la stazione iniziale un solo arco può uscire
									for(unsigned int m=0;m<N_MACCHINE;m++){											
											e_vincolo_partenza_veicolo[m]+=v_cliente[m][current_edge];
											if(i==(posizione_iniziale_veicolo[m]-1))	{			//per ogni macchina inserisco solo posizione iniziale
												e_vincolo_posiz_iniziale_veicolo[m]+=v_cliente[m][current_edge];
											}
									}
								}
								else
									for(unsigned int m=0;m<N_MACCHINE;m++)
										e_cons_flusso_veicolo[m][current_vertex]+=v_cliente[m][current_edge];					
							}																	
							if(target(*ei,g)==current_vertex)
								if(j!=N_TIME_STEPS-1)
									for(unsigned int m=0;m<N_MACCHINE;m++)
										e_cons_flusso_veicolo[m][current_vertex]-=v_cliente[m][current_edge];
						}
					}	
					for(unsigned int m=0;m<N_MACCHINE;m++){
						if ( time_step[current_vertex]!=1 && time_step[current_vertex]!=N_TIME_STEPS) {		//vincolo non valido per istante iniziale e finale

						nn	<< "rmac " << m <<" vert "<<station_name[current_vertex]<<" t "<< time_step[current_vertex]; //constraint name


						IloRange cons_flusso_veicolo(env,0,e_cons_flusso_veicolo[m][current_vertex],0,nn.str().c_str());
						model.add(cons_flusso_veicolo); 


						nn.clear();
						nn.str(std::string());

							if(stampa==1)
								std::cout	<<"Staz "<<i+1<<" tempo "<<j+1<< " macch "<<m<<" : "
											<< cons_flusso_veicolo<<"\n";
						}

					}	
		}
	}
	std::cout<<std::endl;
	std::cout	<<"Vincolo posizione partenza:" <<'\n';
	for(unsigned int m=0;m<N_MACCHINE;m++){
nn	<< "posiz iniz m " << m ;

		model.add(IloRange(env,1,e_vincolo_posiz_iniziale_veicolo[m],1,nn.str().c_str()));		//rispetta posizioni iniziali;disattivare per partenza qualsiasi
			nn.clear();
								nn.str(std::string());


nn	<< "vinc part m " << m ;
	

		model.add(IloRange(env,1,e_vincolo_partenza_veicolo[m],1,nn.str().c_str()));				//Nel primo time step numero archi uscenti=N_MACCHINE (posizione qualsiasi)
	nn.clear();
								nn.str(std::string());


	}

	for(unsigned int m=0;m<N_MACCHINE;m++){
	e_cons_flusso_veicolo[m].clear();
	}
	
	e_vincolo_posiz_iniziale_veicolo.clear();
	e_vincolo_partenza_veicolo.clear();	
	e_cons_flusso_veicolo.clear();

	e_vincolo_posiz_iniziale_veicolo.end();
	e_vincolo_partenza_veicolo.end();	
	e_cons_flusso_veicolo.end();
	
	std::cout<<std::endl;



	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato creaz vincoli flusso part veicoli: "<<elapsed.count()<<std::endl;
	start = std::chrono::high_resolution_clock::now();

	
//Vincoli: conservazione flusso + nodo origine per relocators
	std::cout	<<"Vincolo flusso relocators: "<<'\n';
    for (unsigned int i=0; i<N_STAZIONI;i++){
    	for (unsigned int j=0; j<N_TIME_STEPS;j++){
			for(unsigned int r=0;r<N_RELOCATORS;r++) 
				e_cons_flusso_relocator[r].add(IloExpr(env));
    			current_vertex=map_vertici[std::make_pair(i+1,j+1)];										
    			boost::tie(ei, ei_end) = edges(g);														
    			    for( ; ei != ei_end; ++ei)	{		
						current_edge=map_edges2[*ei];
						if(tipo_arco[*ei]==TRANSFER || tipo_arco[*ei]==RELOCATION || tipo_arco[*ei]==WAIT){							
    			    	    if(source(*ei,g)==current_vertex){									
								if(j==0)	{										//Per la stazione iniziale un solo arco può uscire
									for(unsigned int r=0;r<N_RELOCATORS;r++){										
										e_vincolo_partenza_relocator[r]+=v_relocator[r][current_edge]; //PROBLEMA SUL CURR EDGE
										if(i==(posizione_iniziale_relocator[r]-1))	{			//per ogni macchina inserisco solo posizione iniziale
											e_vincolo_posiz_iniziale_relocator[r]+=v_relocator[r][current_edge];
										}
									}
								}
								else
									for(unsigned int r=0;r<N_RELOCATORS;r++)
										e_cons_flusso_relocator[r][current_vertex]+=v_relocator[r][current_edge];
							}																	
							if(target(*ei,g)==current_vertex)
								if(j!=N_TIME_STEPS-1)
									for(unsigned int r=0;r<N_RELOCATORS;r++)
										e_cons_flusso_relocator[r][current_vertex]-=v_relocator[r][current_edge];
						}
					}	
					for(unsigned int r=0;r<N_RELOCATORS;r++){
						if ( time_step[current_vertex]!=1 && time_step[current_vertex]!=N_TIME_STEPS) {		//vincolo non valido per istante iniziale e finale
							



nn	<< "rel " << r <<" vert "<<station_name[current_vertex]<<" t "<< time_step[current_vertex];
											
								
							IloRange cons_flusso_relocator(env,0,e_cons_flusso_relocator[r][current_vertex],0,nn.str().c_str());
							model.add(cons_flusso_relocator); 
								


nn.clear();
								nn.str(std::string());





							if(stampa==1)
								std::cout	<<"Staz "<<i+1<<" tempo "<<j+1<<" : "
											<< cons_flusso_relocator<<"\n";
						}
					}	
		}
	}
	std::cout<<std::endl;	
	std::cout	<<"Vincolo posizione partenza:" <<'\n';
	for(unsigned int r=0;r<N_RELOCATORS;r++){
nn	<< "vinc posiz iniz r " << r ;
		
	model.add(IloRange(env,1,e_vincolo_posiz_iniziale_relocator[r],1,nn.str().c_str()));		//rispetta posizioni iniziali;disattivare per partenza qualsiasi
		nn.clear();
								nn.str(std::string());


nn	<< "vinc part r " << r ;
	

		model.add(IloRange(env,1,e_vincolo_partenza_relocator[r],1,nn.str().c_str()));				//Nel primo time step numero archi uscenti=N_MACCHINE (posizione qualsiasi)
	nn.clear();
								nn.str(std::string());
	
	}

	for(unsigned int r=0;r<N_RELOCATORS;r++){
		e_cons_flusso_relocator[r].clear();
	}

	e_vincolo_posiz_iniziale_relocator.clear();
	e_vincolo_partenza_relocator.clear();	
	e_cons_flusso_relocator.clear();

	e_vincolo_posiz_iniziale_relocator.end();
	e_vincolo_partenza_relocator.end();	
	e_cons_flusso_relocator.end();
	std::cout<<std::endl;


	
	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato creaz vincoli flusso part relocat: "<<elapsed.count()<<std::endl;
	start = std::chrono::high_resolution_clock::now();


	//vincolo relocator per car
	int current_r_edge=0;
	boost::tie(ei, ei_end) = edges(g);														
    for( ; ei != ei_end; ++ei)	{		
		current_edge=map_edges2[*ei];
		if(tipo_arco[*ei]==RELOCATION ){
			e_relocator_for_car.add(IloExpr(env));
			e_car_for_relocator.add(IloExpr(env));
			for(unsigned int m=0;m<N_MACCHINE;m++){
				e_relocator_for_car[current_r_edge]+=v_cliente[m][current_edge];
				e_car_for_relocator[current_r_edge]-=B*(v_cliente[m][current_edge]);
			}
			for(unsigned int r=0;r<N_RELOCATORS;r++){
				e_relocator_for_car[current_r_edge]-=v_relocator[r][current_edge];
				e_car_for_relocator[current_r_edge]+=v_relocator[r][current_edge];
			}
			model.add(IloRange(env,-IloInfinity,e_relocator_for_car[current_r_edge],0));
			if(stampa==1)
				std::cout<<"Vincolo relocator for car per edge "<< current_edge <<" : " << IloRange(env,-IloInfinity,e_relocator_for_car[current_r_edge],0) <<'\n';
			model.add(IloRange(env,-IloInfinity,e_car_for_relocator[current_r_edge],0));
			if(stampa==1)
				std::cout<<"Vincolo car for relocator per edge "<< current_edge <<" : " << IloRange(env,-IloInfinity,e_car_for_relocator[current_r_edge],0) <<'\n';
			current_r_edge++;
		}
	}
	e_relocator_for_car.clear();
	e_car_for_relocator.clear();

	e_relocator_for_car.end();
	e_car_for_relocator.end();
	std::cout<<std::endl;



	std::cout<<"\n========================================================================================\nBEGIN HEURISTIC\n"<<std::endl;
	for(unsigned int i=0; i<map_edges.size();i++){				
    	edge=map_edges[i].first;	
		for(unsigned int m=0;m<N_MACCHINE;m++){
			if ( tipo_arco[edge]==RELOCATION){
				v_cliente[m][i].setBounds(0,0);
			}
		}
		for(unsigned int r=0;r<N_RELOCATORS;r++){
			if ( tipo_arco[edge]==RELOCATION||tipo_arco[edge]==TRANSFER){
				v_relocator[r][i].setBounds(0,0);
			}
		

		}
	}

	
	cplex.extract(model);	

	
	

	cplex.setParam(IloCplex::Param::Threads, 1);
	cplex.setParam(IloCplex::Param::TimeLimit, CPLEX_TIME);
   	 cplex.solve () ;

	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato cplex: "<<elapsed.count()<<std::endl;

 	std::cout<<" Solution status= "	<<	cplex.getStatus()			<<std::endl;
    	std::cout<<" Solution value=  "	<<	cplex.getObjValue()		<<std::endl;
	std::cout<<" Best bound=  "	<<	cplex.getBestObjValue()		<<std::endl;
	std::cout<<" Gap=  "		<<	cplex.getMIPRelativeGap()		<<std::endl;
	std::cout<<" Node number=  "	<<	cplex.getNnodes()			<<std::endl;
	std::cout<<" Col number=  "	<<	cplex.getNcols()			<<std::endl;
	std::cout<<" Row number=  "	<<	cplex.getNrows()			<<std::endl;
	std::cout<<std::endl;
	
	int chiave=rand() %10000;
	std::string mipnames="mip_"+to_string(chiave)+".mst";
	const char* mipname=mipnames.c_str();
	cplex.writeMIPStarts(mipname);





	
	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato creaz ultimi 2 vincoli: "<<elapsed.count()<<std::endl;
	start = std::chrono::high_resolution_clock::now();


int durata_intervallo = N_TIME_STEPS/INTERVALLI;
IloArray <IloIntArray> current_solution_r(env);
IloArray <IloIntArray> current_solution_m(env) ;
double x;
	
for(unsigned int m=0;m<N_MACCHINE;m++){
			current_solution_m.add(IloIntArray(env));
			for(unsigned int i=0; i<map_edges.size();i++){
				
				current_solution_m[m].add(100);
			}
	}
	
		for(unsigned int r=0;r<N_RELOCATORS;r++){
			current_solution_r.add(IloIntArray(env));

			for(unsigned int i=0; i<map_edges.size();i++){
	
				current_solution_r[r].add(100);
			}
	}



 auto inizio_cplex = std::chrono::high_resolution_clock::now();
for (int intervallo=1;intervallo<=INTERVALLI;intervallo++){

	std::cout<<"\n\n========================================================================================\nBEGIN ROLLING "<<intervallo<<std::endl;

	//insert again relocation arcs from 0 to T/INTERVALLI by setting their bounds to 0,1

	for(unsigned int i=0; i<map_edges.size();i++){				
    	edge=map_edges[i].first;
		current_edge=map_edges2[edge];
		current_edge_source_time=time_step[map_edges[current_edge].second.first];
		if(current_edge_source_time<intervallo*durata_intervallo && current_edge_source_time>=(intervallo-1)*durata_intervallo){
		for(unsigned int m=0;m<N_MACCHINE;m++){
			if ( tipo_arco[edge]==RELOCATION)
				v_cliente[m][i].setBounds(0,1);
		}
		for(unsigned int r=0;r<N_RELOCATORS;r++){
			if ( tipo_arco[edge]==RELOCATION||tipo_arco[edge]==TRANSFER)
				v_relocator[r][i].setBounds(0,1);
		}	
		}
	}



	cplex.extract(model);	
	

	if (intervallo==1){
	cplex.readMIPStarts(mipname);
	cplex.setParam(IloCplex::MIPDisplay, 4);
	remove(mipname);
	cplex.setParam(IloCplex::Param::TimeLimit, CPLEX_TIME);
	}
		
	cplex.solve () ;
	
	//save current solution (all of it, even after intervallo*durata_intervallo)

	double u;
		for(unsigned int m=0;m<N_MACCHINE;m++){
			for(unsigned int i=0; i<map_edges.size();i++){
				current_solution_m[m][i]=cplex.getValue(v_cliente[m][i]);
				edge=map_edges[i].first;
		current_edge=map_edges2[edge];
		current_edge_source_time=time_step[map_edges[current_edge].second.first];
		current_edge_target_time=time_step[map_edges[current_edge].second.second];
			
			if(m==2 && current_edge_target_time==22 && current_edge_source_time==15){
				u=cplex.getValue(v_cliente[m][i]);
				std::cout<<" M2 da 15 a 22 da vettore solution "<< current_solution_m[m][i]<< " e da get value  "<< u <<'\n';
				
			}

			}
	}
	
		for(unsigned int r=0;r<N_RELOCATORS;r++){
			for(unsigned int i=0; i<map_edges.size();i++){
				current_solution_r[r][i]=cplex.getValue(v_relocator[r][i]);
			}
	}


	


	//insert previous solution only for arcs until intervallo*durata_intervallo; integrality with EPSILON
	
	
	

	if(intervallo!=INTERVALLI) {
	for(unsigned int i=0; i<map_edges.size();i++){
				
    	edge=map_edges[i].first;
		current_edge=map_edges2[edge];
		current_edge_source_time=time_step[map_edges[current_edge].second.first];
		if(current_edge_source_time<intervallo*durata_intervallo && current_edge_source_time>=(intervallo-1)*durata_intervallo){

		for(unsigned int m=0;m<N_MACCHINE;m++){
			x=current_solution_m[m][i];
			v_cliente[m][i].setBounds(IloRound(x),IloRound(x));

		}
		for(unsigned int r=0;r<N_RELOCATORS;r++){
			x=current_solution_r[r][i];
			v_relocator[r][i].setBounds(IloRound(x),IloRound(x));
		}
		}
		
	}
	}

	for(unsigned int i=0; i<map_edges.size();i++){
   edge=map_edges[i].first;
current_edge=map_edges2[edge];
current_edge_source_time=time_step[map_edges[current_edge].second.first];
current_edge_target_time=time_step[map_edges[current_edge].second.second];
	for(unsigned int r=0;r<N_RELOCATORS;r++){
		if(current_solution_r[r][i]!=0 && current_edge_target_time<=15 )	
	std::cout<<" CHI RELOCA PRIMA DI 15 arco "<< current_solution_r[r][i]<< " rel  "<<r <<" tipo "<<tipo_arco[edge]<< " partenza "<< current_edge_source_time<<" arrivo  "<< current_edge_target_time<<'\n';

	}


		for(unsigned int m=0;m<N_MACCHINE;m++){

if(current_solution_m[m][i]!=0 && current_edge_target_time==22 && current_edge_source_time==15)	
std::cout<<" CHI PERCORRE DA 15 a 22 arco "<< current_solution_m[m][i]<< " macch "<<m<<" tipo "<<tipo_arco[edge]<< " partenza "<< current_edge_source_time<<" arrivo  "<< current_edge_target_time<<'\n';

if(current_solution_m[m][i]!=0 && current_edge_source_time==15)	
std::cout<<" CHI PERCORRE DA 15 arco "<< current_solution_m[m][i]<< " macch "<<m<<" tipo "<<tipo_arco[edge]<< " partenza "<< current_edge_source_time<<" arrivo  "<< current_edge_target_time<<'\n';

//if(current_solution_m[m][i]!=0 && tipo_arco[edge]!=0 )	
//std::cout<<"arco "<< current_solution_m[m][i]<< " macch "<<m<<" tipo "<<tipo_arco[edge]<< " partenza "<< current_edge_source_time<<" arrivo  "<< current_edge_target_time<<'\n';


if(m==2 && current_solution_m[m][i]!=0 )	
std::cout<<"arco "<< current_solution_m[m][i]<< " MACCH "<<m<<" tipo "<<tipo_arco[edge]<< " partenza "<< current_edge_source_time<<" arrivo  "<< current_edge_target_time<<'\n';



}
}
std::cout<<std::endl;
	
				
}


//end of rolling




	end = std::chrono::high_resolution_clock::now();
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout<<"Tempo impiegato cplex: "<<elapsed.count()<<std::endl;

    	std::cout<<" Solution status= "		<<	cplex.getStatus()			<<std::endl;
    	std::cout<<" Solution value=  "		<<	cplex.getObjValue()		<<std::endl;
	std::cout<<" Best bound=  "		<<	cplex.getBestObjValue()		<<std::endl;
	std::cout<<" Gap=  "			<<	cplex.getMIPRelativeGap()		<<std::endl;
	std::cout<<" Node number=  "		<<	cplex.getNnodes()			<<std::endl;
	std::cout<<" Col number=  "		<<	cplex.getNcols()			<<std::endl;
	std::cout<<" Row number=  "		<<	cplex.getNrows()			<<std::endl;
	std::cout<<std::endl;

	

	std::cout	<<"\n\n\n------------------------------------------------------------------\n\n\nISTANZA "
				<<"TEMPO "	<<"\tSTAZIONI "	<<"\tTIME STEPS "	<<"\tMACCHINE "	<<"\tRELOCATORS "	<<"\tAC "	<<"\tSOL "<<"\tGAP "<<"\tBOUND "<<"\tNODi "	<<"\tCOLONNE "	<<"\tRIGHE "	<<"\tT NODO 0 "<<"\tSOLUZ NODO 0"<<"\tBOUND NODO 0"<<"\tTEMPO NODO 0"<<"\tGAP NODO 0"	<<std::endl;
	std::cout		<<"\t"<<argv[1]		<<"\t"<<argv[2]	<<"\t"<<argv[3]		<<"\t"<<AC;

	std::cout	<<"\t"<<cplex.getObjValue()		<<"\t"<<cplex.getMIPRelativeGap()	<<"\t"<<cplex.getBestObjValue()	<<"\t"<<cplex.getNnodes()	<<"\t"<<cplex.getNcols()	<<" \t"<<cplex.getNrows()	<<" \t"<< elapsed.count()/1000<<" \t"<<std::endl;

	

	
	

} catch(IloException& e){
    std::cerr << "Concert exception caught: " << e << std::endl;
	std::cerr <<e.getMessage();
	e.end();
}



env.end();




//CString file_grafo="c:\\Users\\Filippo\\Documents\\Visual Studio 2012\\Projects\\grafo3\\grafo3\\grafo.dot";
//ShellExecute(NULL,NULL,file_grafo,NULL,NULL,SW_SHOW);
//CString file_output="c:\\Users\\Filippo\\Documents\\Visual Studio 2012\\Projects\\grafo3\\grafo3\\output.txt";
//ShellExecute(NULL,NULL,file_output,NULL,NULL,SW_SHOW);

return 0;

}

