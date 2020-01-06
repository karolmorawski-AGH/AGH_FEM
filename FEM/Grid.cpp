#include "Grid.h"

Grid::Grid(GlobalData data)
{
	double H = data.getH();
	double W = data.getW();
	uint nH = data.getNH();
	uint nW = data.getNW();
	uint nN = data.getNN();
	uint nE = data.getNE();
	double mTbegin = data.getTemp();
	
	/*Creating nodes*/
	double xsegmnt = W / (nW-1);
	double ysegmnt = H / (nH-1);
	
	double xgridref = 0;
	double ygridref = 0;

	for (int x = 0; x < nW; x++) {
		xgridref = x * xsegmnt;
		for (int y = 0; y < nH; y++) {
			ygridref = y * ysegmnt;
			
			bool bc = 0;
			if (xgridref == 0 || ygridref == 0 || xgridref == W || ygridref == H)
				bc = 1;

			Node* node = new Node(xgridref, ygridref, mTbegin, bc);
			nodes.push_back(node);
		}
	}

	/*Creating elements (4 nodes per element)*/
	int k = 0;
	for (int i = 0; i < nE; i++) {

		if ((i % nH) == (nH - 1)) {
			nE++;
			continue;
		}

		Node* a = nodes[i];
		Node* b = nodes[i + 1];
		Node* c = nodes[i + nH];
		Node* d = nodes[i + nH + 1];

		//TODO
		//Na tym etapie niepoprawne BC
		Element* element = new Element(a,b,c,d);
		elements.push_back(element);

	}

	// Sets boundary condition for each element
	this->set_boundary_cond();
}

Grid::Grid()
{
}

Grid::~Grid()
{
	for (int i = 0; i < elements.size(); i++) {
		delete elements[i];
	}
}

void Grid::print_nodes()
{
	for (int i = 0; i < nodes.size(); i++) {
		std::cout << i+1 << ". ";
		nodes[i]->print();
	}
}

void Grid::print_elements()
{
	for (int i = 0; i < elements.size(); i++) {
		std::cout << "Element no." << i + 1 << ". " << std::endl;
		elements[i]->print();
		std::cout << std::endl;
	}
}

Element* Grid::getElement(int id)
{
	return elements[id];
}

void Grid::set_boundary_cond()
{
	int node_stat[4] = { 0, 0, 0, 0 };
	for (int i = 0; i < elements.size(); i++) {
		Element* element = this->getElement(i);

		for (int j = 0; j < 4; j++) {
			node_stat[j] = element->getNode(j).getBC();
		}

		//zalozmy ze od boku dolnego counterclockwise do boku lewego
		//1-dolny, 2-prawy, 3-gorny, 4-lewy
		//bok dolny
		if (node_stat[0] == 1 && node_stat[1] == 1) {
			if (element->getEdgeCount() == 0) {
				element->updateEdgeList(0, 1);
				element->setEdgeCount(1);
			}
			else {
				element->updateEdgeList(1, 1);
				element->setEdgeCount(2);
			}
		}
		//bok prawy
		if (node_stat[1] == 1 && node_stat[2] == 1) {
			if (element->getEdgeCount() == 0) {
				element->updateEdgeList(0, 2);
				element->setEdgeCount(1);
			}
			else {
				element->updateEdgeList(1, 2);
				element->setEdgeCount(2);
			}
		}
		//bok gorny
		if (node_stat[2] == 1 && node_stat[3] == 1) {
			if (element->getEdgeCount() == 0) {
				element->updateEdgeList(0, 3);
				element->setEdgeCount(1);
			}
			else {
				element->updateEdgeList(1, 3);
				element->setEdgeCount(2);
			}
		}
		//bok lewy
		if (node_stat[3] == 1 && node_stat[4] == 1) {
			if (element->getEdgeCount() == 0) {
				element->updateEdgeList(0, 4);
				element->setEdgeCount(1);
			}
			else {
				element->updateEdgeList(1, 4);
				element->setEdgeCount(2);
			}
		}
	}
}

void Grid::compute(int nH, int nW, double specificHeat, double density, double lambda, double tstep, double alfa, double t_ambient)
{
	UniversalElement* uElem = new UniversalElement(4, 4);

	//this->print_elements();

	//Global H matrix
	std::vector<double> pGlobal;
	pGlobal.resize(nH * nW);
	for (int i = 0; i < nH * nW; i++) {
		pGlobal[i] = 0.0;
	}

	//Global P vector
	std::vector<std::vector<double>> hGlobal;
	hGlobal.resize(nH * nW);
	for (int i = 0; i < nH * nW; i++) {
		hGlobal[i].resize(nH * nW);
		for (int j = 0; j < nH * nW; j++) {
			hGlobal[i][j] = 0.0;
		}
	}


	double dNdx[4];
	double dNdy[4];
	double coordX[4];
	double coordY[4];
	double initialTemp[4];
	double tempInt = 0.;
	double det = 0.;
	//double cMatrix[4][4];

	//Local H matrix, P vector and C matrix
	double hLocal[4][4] = { {0.,0.,0.,0.}, {0.,0.,0.,0.}, {0.,0.,0.,0.}, {0.,0.,0.,0.} };
	double hsLocal[4][4] = { {0.,0.,0.,0.}, {0.,0.,0.,0.}, {0.,0.,0.,0.}, {0.,0.,0.,0.} };
	double cLocal[4][4] = { {0.,0.,0.,0.}, {0.,0.,0.,0.}, {0.,0.,0.,0.}, {0.,0.,0.,0.} };
	double pLocal[4] = { 0.,0.,0.,0. };
	// Iterates through each element (computing H and P)
	for (int i = 0; i < this->elements.size(); i++) {
		Element *localElement = this->getElement(i);

		// Gets all nodes parameters from element
		for (int j = 0; j < 4; j++) {
			Node localNode = localElement->getNode(j);
			coordX[j] = localNode.getX();
			coordY[j] = localNode.getY();
			initialTemp[j] = localNode.getTemp();
		}
		
		// Tworzy macierze jakobiego i wyznacza pochodne funkcji ksztaltu po x/y + interpolacja temp
		for (int j = 0; j < 4; j++) {
			Jacobian* jacobian = new Jacobian(coordX, coordY, j, *uElem);
			tempInt = 0.;

			for (int k = 0; k < 4; k++) {

				dNdx[k] = jacobian->getInvertedJacobian()[0][0] * uElem->getKsiMatrix()[j][k]
						+ jacobian->getInvertedJacobian()[0][1] * uElem->getEtaMatrix()[j][k];

				dNdy[k] = jacobian->getInvertedJacobian()[1][0] * uElem->getKsiMatrix()[j][k]
						+ jacobian->getInvertedJacobian()[1][1] * uElem->getEtaMatrix()[j][k];

			}

				det = jacobian->getDet();

				// N x N^T
				// Calka objetosciowa do H i C
				for (int k = 0; k < 4; k++) {
					for (int l = 0; l < 4; l++) {
						// C lokalne - OK
						cLocal[k][l] = specificHeat * density * uElem->getSVMatrix()[j][k] * uElem->getSVMatrix()[j][l] * det;
						// H lokalne - OK 
						hLocal[k][l] = lambda * (dNdx[k] * dNdx[l] + dNdy[k] * dNdy[l]) * det;// +cLocal[k][l] / tstep;
					}
				}
			}
		
		// TODO
		/*
		for (int k = 0; k < 4; k++) {
			for (int l = 0; l < 4; l++) {
				// demo
				if (i == 4) {
					std::cout << hLocal[k][l] << std::endl;
				}
			}
		}
		*/

		//Calka powierzchniowa do H i wektor P
		for (int n_surf = 0; n_surf < localElement->getEdgeCount(); n_surf++) {
			
			//FIXME 
			uint surface_id = localElement->getEdgeList()[n_surf];
			double edge_length, surf_det, x_cords[4], y_cords[4], shape_func[2];;

			// Wspolrzedne elementu skonczonego
			for (int cords = 0; cords < 4; cords++) {
				x_cords[cords] = localElement->getNode(cords).getX();
				y_cords[cords] = localElement->getNode(cords).getY();
			}

			// Obliczanie dlugosci boku
			// Kolejnosc jest najprawdopodobniej zla, ale wystarczy podmienic indeksy (sal 1)
			if (surface_id == 0) {
				edge_length = sqrt(pow((x_cords[1]-x_cords[0]), 2) + pow((y_cords[1] - y_cords[0]),2));
			}
			else if (surface_id == 1) {
				edge_length = sqrt(pow((x_cords[1] - x_cords[2]), 2) + pow((y_cords[1] - y_cords[2]), 2));
			}
			else if (surface_id == 2) {
				edge_length = sqrt(pow((x_cords[2] - x_cords[3]), 2) + pow((y_cords[2] - y_cords[3]), 2));
			}
			else {
				edge_length = sqrt(pow((x_cords[3] - x_cords[0]), 2) + pow((y_cords[3] - y_cords[0]), 2));
			}

			// lokalny wyznacznik macierzy jakobiego
			surf_det = edge_length / 2.;

			// Dla 2 pkt calkowania
			for (int i_point = 0; i_point < 2; i_point++) {
				for (int j = 0; j < 4; j++) {
					for (int k = 0; k < 4; k++) {
						// TODO
						// Wartosci funkcji ksztaltu dla scian powierzchniowych
						shape_func[0] = 0.0;
						shape_func[1] = 0.0;

						// Macierz H po powierzchni
						hsLocal[j][k] += alfa * shape_func[0] * shape_func[1] * surf_det;
					}
					pLocal[j] += alfa * shape_func[0] * surf_det * t_ambient;
				}

			}

		}

		// TODO 
		// Dodac hLocal i hsLocal
		// Agregacja do macierzy wynikowej hlocal+hslocal plocal i clocal
		for (int j = 0; j < 4; j++) {
		
			for (int k = 0; k < 4; k++) {

			}
		}
	}



}

