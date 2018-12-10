#include "stdafx.h"

#define BOUNDARY 0.0001

SPHSystem::SPHSystem() {
	//std::cout << "construct sph" << std::endl;
	pNum = 0; 
	//pNumMax = 30000; 
	pNumMax = 6000;
	particles = new Particle[pNumMax];	
	
	h = 0.04; // radius of kernel
	h2 = h * h; // kernel_2: kernel * kernel
	mass = 0.02;

	worldSize.x = 0.64; worldSize.y = 0.64; worldSize.z = 0.64; 
	cellSize = h; 
	gridSize.x = (unsigned int)ceil(worldSize.x / h);	gridSize.y = (unsigned int)ceil(worldSize.y / h);	gridSize.z = (unsigned int)ceil(worldSize.z / h);//grid_size, 16*16*16
	cellNum = gridSize.x * gridSize.y * gridSize.z; // 4096
	cell = new Particle*[cellNum];
	//std::cout << "gridSize: (" << gridSize.x << ", " << gridSize.y << ", " << gridSize.z << " ), cellNum: " << cellNum << std::endl;

	

	//timeStep = 0.003;
	timeStep = 0.005;
	gravity.x = 0.0; gravity.y = - 6.8; gravity.z = 0.0;

	wallDamping = -0.5f; 
	//wallDamping = -0.8f;
	restDens = 1000.0; 
	restDens1 = 1000.0;
	restDens2 = 1500.0;
	restDens3 = 2000.0;
	R = 1.0; 
	viscosity = 6.5;
	surfNorm = 6.0; //  surface tension tipping point
	surfCoe = 0.1; // coefficient

	//surfNorm = 15.0; //  surface tension tipping point
	//surfCoe = 0.5; // coefficient

	poly6 = 315.0 / (64.0 * PI * pow(h,9)); 
	spiky = -45.0 / ( PI * pow(h,6)); 
	visco = 45.0 / ( PI * pow(h,6)); 

	poly6Grad = -945 / ( 32 * PI * pow(h, 9)); // gradient 
	poly6Lapl = -945 / ( 8 * PI * pow(h, 9)); // laplace 

	kDens = mass * poly6 * pow(h,6); 
	kColorLapl = mass * poly6Lapl * h2 * (0 - 3 / 4 * h2); 

}

SPHSystem::~SPHSystem() {
	//std::cout << "deconstruct sph" << std::endl;
	delete[]particles;
	delete[]cell;
}

void SPHSystem::init() {
	//std::cout << " - initial particles" << std::endl;

	double initRange = 0.6;
	//double initRange = 0.4;
	double delta = h * 0.5;
	//double delta = h * 0.4;

	Particle *p;

	for (double x = 0.0; x < worldSize.x * initRange; x += delta) {
		if (pNum >= pNumMax)	break;
		for (double y = 0.0; y < worldSize.y * initRange; y += delta) {
			if (pNum >= pNumMax)	break;
			for (double z = 0.0; z < worldSize.z * initRange; z += delta) {
				if (pNum >= pNumMax)	break;
				//std::cout << "x: " << x << "y: " << y << "z: " << z << std::endl;
				// add particle in particle list
				p = &(particles[pNum]);

				p->id = pNum;

				p->pos.x = x;	p->pos.y = y;	p->pos.z = z;
				p->vel.x = 0.0; p->vel.y = 0.0; p->vel.z = 0.0;
				
				if (test_density == 0)
					p->dens = restDens;
				if (test_density == 1)
				{
					if (pNum < 1000)
						p->dens = restDens1;
					else if (pNum < 2000)
						p->dens = restDens2;
					else if (pNum < 3000)
						p->dens = restDens3;
				}
				p->press = 0.0;

				p->acc.x = 0.0; p->acc.y = 0.0; p->acc.z = 0.0;

				p->next = NULL;

				++pNum;
			}
		}
	}

	if (pNum < pNumMax) pNumMax = pNum;

}

void SPHSystem::run() {
	//std::cout << " - sph try run" << std::endl;
	if (sysRunning) {
		buildTable();
		calcDensPress();
		calcForceAdv();
		update();
	}
}

void SPHSystem::buildTable() {
	//std::cout << " -- built cell hash table" << std::endl;
	Particle *p;
	unsigned int hash;

	for (unsigned int i = 0; i < cellNum; i++) {
		cell[i] = NULL;
	}

	for (unsigned int i = 0; i < pNum; i++) {
		p = &(particles[i]);

		// get position in grid(cell) of particle
		p->cellPos.x = int(floor(p->pos.x / cellSize));
		p->cellPos.y = int(floor(p->pos.y / cellSize));
		p->cellPos.z = int(floor(p->pos.z / cellSize));

		hash = calCellHash(p->cellPos);
		//std::cout << " --- cell hash: " << hash << std::endl;
		
		if (cell[hash] == NULL) {
			p->next = NULL;
		}
		else {
			p->next = cell[hash];
		}
		cell[hash] = p;

	}

	
}

void SPHSystem::calcDensPress() {
	Particle *p;
	Particle *np; //neighbour

	int3 nPos; // neighbour pos
	unsigned int hash;

	Vector3D deltaPos; // p->pos - np->pos

	double r2; // r^2

	// traverse every particle
	for (unsigned int i = 0; i < pNum; i++) {
		p = &(particles[i]);

		p->dens = 0.0;
		p->press = 0.0;

		// traverse all neighbour cells
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				for (int z = -1; z <= 1; z++) {
					// get all particles in a neighbour cell
					nPos.x = p->cellPos.x + x;
					nPos.y = p->cellPos.y + y;
					nPos.z = p->cellPos.z + z;
					hash = calCellHash(nPos);
					if (hash == 0xffffffff)	continue;
					np = cell[hash];

					// traverse all particles in a cell
					while (np != NULL) {
						deltaPos = p->pos - np->pos;
						r2 = deltaPos.norm2();

						if ( r2<h2 && r2>EPS_D ) {
							p->dens += mass * poly6 * pow(h2 - r2, 3);
						}
						np = np->next;
					} // end while: traverse all particles in a cell

				}
			}
		} // end for(for(for())): traverse all neighbour cells

		p->dens = p->dens + kDens;

		

		//Regarding density
		if (test_density == 0)
			p->press = (pow(p->dens / restDens, 7) - 1) * R; // Tait equation
		if (test_density == 1)
		{
			if (i < 1000)
				p->press = (pow(p->dens / restDens1, 7) - 1) * R; // Tait equation
			else if (i < 2000)
				p->press = (pow(p->dens / restDens2, 7) - 1) * R; // Tait equation
			else if (i < 3000)
				p->press = (pow(p->dens / restDens3, 7) - 1) * R; // Tait equation
		}
	} // end for: traverse every particle
}


void SPHSystem::calcForceAdv() {
	Particle *p;
	Particle *np; //neighbour

	int3 nPos; // neighbour pos
	unsigned int hash;

	Vector3D deltaPos; // p->pos - np->pos
	Vector3D deltaVel; // np->vel - p->vel

	double r; // radius between 2 particles
	double r2; // r^2
	double h_r; // kernel radius(h) - r

	// surface tension
	Vector3D colorGrad; 
	double colorLapl; 

	for (unsigned int i = 0; i < pNum; i++) {
		p = &(particles[i]);

		// init accelerate and color gradient and laplace
		for (int j = 0; j < 3; j++) { 
			p->acc[j] = 0.0;
			colorGrad[j] = 0.0;
		}
		colorLapl = 0.0;

		// traverse all neighbour cells
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				for (int z = -1; z <= 1; z++) {
					// get all particles in a neighbour cell
					nPos.x = p->cellPos.x + x;
					nPos.y = p->cellPos.y + y;
					nPos.z = p->cellPos.z + z;
					hash = calCellHash(nPos);
					if (hash == 0xffffffff)	continue;
					np = cell[hash];

					// traverse all particles in a cell
					while (np != NULL) {
						deltaPos = p->pos - np->pos;
						r2 = deltaPos.norm2();

						if ( r2<h2 && r2>EPS_D ){
							// These parameters would be used in calculating acceleration of both pressure and viscosity
							r = deltaPos.norm();
							h_r = h - r;
							double temp = mass / (2 * np->dens * p->dens);

							// acceleration of preesure
							p->acc -= temp * spiky * (p->press + np->press) * h_r * h_r * deltaPos / deltaPos.norm();

							// acceleration of viscosity
							deltaVel = np->vel - p->vel;
							p->acc += temp * viscosity * visco * deltaVel * h_r;

							colorGrad += (-1) * poly6Grad * (mass / np->dens) * pow(h2 - r2, 2) * deltaPos;
							colorLapl += poly6Lapl * ( mass / np->dens ) * (h2 - r2) * (r2 - 3 / 4 * (h2 - r2));
						}
						np = np->next;
					} // end whild: traverse all particles in a cell

				}
			}
		} // end for(for(for())): get all particles in a neighbour cell

		// acceleration of surface tension
		colorLapl += kColorLapl;
		//if ( p->surfNorm > surfNorm) {
		if (1) {
			p->acc += surfCoe * colorLapl * colorGrad / ( p->dens * colorGrad.norm());
		}

	} // end for: traverse every particle

}

void SPHSystem::update() {
	//std::cout << " -- update velocity and position" << std::endl;
	Particle *p;
	for (unsigned int i=0; i < pNum;i++) {
		p = &(particles[i]);

		p->vel += p->acc * timeStep + gravity * timeStep;

		p->pos += p->vel * timeStep;

		// Bounding from the boundary
		for (int j = 0; j < 3;j++) {
			if (p->pos[j] >= worldSize[j] - BOUNDARY) {
				p->vel[j] *= wallDamping;
				p->pos[j] = worldSize[j] - BOUNDARY;
			}
			if (p->pos[j] < 0.0) {
				p->vel[j] *= wallDamping;
				p->pos[j] = 0.0 + BOUNDARY;
			}
		}
	}
}

unsigned int SPHSystem::calCellHash(int3 pos) {
	if (pos.x < 0 || pos.x >= int(gridSize.x) || pos.y < 0 || pos.x >= int(gridSize.y) || pos.z < 0 || pos.z >= int(gridSize.z)) {
		return (unsigned int)0xffffffff;
	}

	pos.x = pos.x & (gridSize.x - 1);
	pos.y = pos.y & (gridSize.y - 1);
	pos.z = pos.z & (gridSize.z - 1);

	return ((unsigned int)(pos.x)) + ((unsigned int)(pos.y)) * gridSize.x + ((unsigned int)(pos.z)) * gridSize.x * gridSize.y;
}