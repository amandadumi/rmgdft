/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
/*** QMD-MGDFT/main.h *****
 * NAME
 *   Ab initio real space code with multigrid acceleration
 *   Quantum molecular dynamics package.
 *   Version: 2.1.5
 * COPYRIGHT
 *   Copyright (C) 1995  Emil Briggs
 *   Copyright (C) 1998  Emil Briggs, Charles Brabec, Mark Wensell, 
 *                       Dan Sullivan, Chris Rapcewicz, Jerzy Bernholc
 *   Copyright (C) 2001  Emil Briggs, Wenchang Lu,
 *                       Marco Buongiorno Nardelli,Charles Brabec, 
 *                       Mark Wensell,Dan Sullivan, Chris Rapcewicz,
 *                       Jerzy Bernholc
 * FUNCTION
 *   
 * INPUTS
 *
 * OUTPUT
 *  
 * PARENTS
 *
 * CHILDREN
 * 
 * SEE ALSO
 *  
 * SOURCE
 */


#if MPI
typedef struct
{
    int count;
} QMD_sem_t;
#endif

/* Processor grid storage on finest level */
typedef struct
{

    REAL b[PX0_GRID][PY0_GRID][PZ0_GRID];

} P0_GRID_S;


/* Here we use a union of P0_GRID_S and a single long real array so that */
/* we can access things in that manner.                              */
typedef union
{

    P0_GRID_S s1;
    REAL s2[PX0_GRID * PY0_GRID * PZ0_GRID];

} P0_GRID;

/* Processor grid storage on finest level on the Fine Grid*/
typedef struct
{

    REAL b[FPX0_GRID][FPY0_GRID][FPZ0_GRID];

} FP0_GRID_S;


/* Here we use a union of FP0_GRID_S and a single long real array so that */
/* we can access things in that manner.                              */
typedef union
{
    FP0_GRID_S s1;
    REAL s2[FPX0_GRID * FPY0_GRID * FPZ0_GRID];

} FP0_GRID;


/* Smoothing grid storage on finest level */
typedef struct
{

    REAL b[PX0_GRID + 2][PY0_GRID + 2][PZ0_GRID + 2];

} S0_GRID_S;


/* Here we use a union of S0_GRID_S and a single long real array so that */
/* we can access things in that manner.                              */
typedef union
{

    S0_GRID_S s1;
    REAL s2[(PX0_GRID + 2) * (PY0_GRID + 2) * (PZ0_GRID + 2)];

} S0_GRID;

/* Smoothing grid storage on finest level on the Fine Grid*/
typedef struct
{

    REAL b[FPX0_GRID + 2][FPY0_GRID + 2][FPZ0_GRID + 2];

} FS0_GRID_S;

/* Here we use a union of FS0_GRID_S and a single long real array so that */
/* we can access things in that manner.                              */
typedef union
{

    FS0_GRID_S s1;
    REAL s2[(FPX0_GRID + 2) * (FPY0_GRID + 2) * (FPZ0_GRID + 2)];

} FS0_GRID;


/* For applying higher order finite difference operators */
typedef struct
{

    REAL b[PX0_GRID + 4][PY0_GRID + 4][PZ0_GRID + 4];

} SS0_GRID;

typedef struct
{

    REAL b[PX0_GRID + 6][PY0_GRID + 6][PZ0_GRID + 6];

} S30_GRID;

typedef struct
{

    REAL b[PX0_GRID + 10][PY0_GRID + 10][PZ0_GRID + 10];

} S50_GRID;


typedef struct
{

    REAL b[FPX0_GRID + 4][FPY0_GRID + 4][FPZ0_GRID + 4];

} FSS0_GRID;


/** @name PE_CONTROL
  *
  * @memo Processor control structure.
  * 
  * This is a global structure declared as extern PE_CONTROL pct.
  * 
 */
typedef struct
{

    /** Number (rank in MPI terminology) of this processor in this image grid */
    int gridpe, imgpe, thisimg, spinpe;

	/** Number of grids (typically 1) per image to be run simultaneously **/
	int images, grids;

	/* MPI communicators for each code grid (grid_comm) and one (rmg_comm)
	 * for all group rank 0 pe's. The later effectively replaces MPI_COMM_WORLD
	 * unless you really need all-to-all, even across grids, communication. */
	MPI_Comm rmg_comm, img_topo_comm, grid_topo_comm, grid_comm, img_comm, spin_comm;



	/* scalapack variables */
	int desca[DLEN];
	int ictxt;

    /*Whether pe participates in scalapack calculations*/
    int scalapack_pe;

    /* Row or Column that given processor handles when using scalapack*/
    int scalapack_myrow;
    int scalapack_mycol;

    /*Processor distribution for scalapack*/
    int scalapack_nprow;
    int scalapack_npcol;


    /** Neighboring processors in three-dimensional space */
    int neighbors[6];

    /** Processor x-coordinate for domain decomposition */
    int pe_x;
    /** Processor y-coordinate for domain decomposition */
    int pe_y;
    /** Processor z-coordinate for domain decomposition */
    int pe_z;


    /** Points to start of projector storage for this ion in projector space */
    //REAL *weight[MAX_IONS];
    REAL **weight;

#if FDIFF_BETA
    /*These are used for non-local force */
    //REAL *weight_derx[MAX_IONS];
    REAL **weight_derx;
    //REAL *weight_dery[MAX_IONS];
    REAL **weight_dery;
    //REAL *weight_derz[MAX_IONS];
    REAL **weight_derz;
#endif


    /** An index array which maps the projectors onto the 3-d grid associated
        with each processor.
    */
    //int *nlindex[MAX_IONS];
    int **nlindex;
    //int *Qindex[MAX_IONS];
    int **Qindex;

    /** An index array which indicate whether the grid map on the current pocessor*/
    //int *idxflag[MAX_IONS];
    int **idxflag;
    //int *Qdvec[MAX_IONS];
    int **Qdvec;

    /** Number of points in the nlindex array for each ion */
    //int idxptrlen[MAX_IONS];
    int *idxptrlen;
    //int Qidxptrlen[MAX_IONS];
    int *Qidxptrlen;

    /** Number of points in the circle of local projector for each pocessor*/
    //int lptrlen[MAX_IONS];
    int *lptrlen;

    /** Phase shifts for the non-local operators */
    //REAL *phaseptr[MAX_IONS];
    REAL **phaseptr;

    /** Number of projectors associated with each ion. */
    //int prj_per_ion[MAX_IONS];
    int *prj_per_ion;

    /** Points to start of storage for theaugument function*/
    //REAL *augfunc[MAX_IONS];
    REAL **augfunc;

    /** points to start of DnmI function storage for this ion*/
    //REAL *dnmI[MAX_IONS];
    REAL **dnmI;

    /** points to start of qqq storage for this ion*/
    //REAL *qqq[MAX_IONS];
    REAL **qqq;

    int num_nonloc_ions;
    int nonloc_ions_list[MAX_NONLOC_IONS];

    int num_nonloc_pes;
    int nonloc_pe_list[MAX_NONLOC_PROCS];
    int nonloc_atom_list_per_pe[MAX_NONLOC_PROCS][MAX_NONLOC_IONS];
    int nonloc_atom_count_per_pe[MAX_NONLOC_IONS];
    
    REAL *oldsintR_local;
    REAL *oldsintI_local;
    REAL *newsintR_local;
    REAL *newsintI_local;

} PE_CONTROL;



/**@name STATE
 *
 * @memo Wavefunction storage structure */
typedef struct
{

    /** First iteration flag */
    int firstflag;

    /** Current estimate of the eigenvalue for this orbital (state). */
    REAL eig[2];

    /** Wavefunction residual error computed by multigrid solver */
    REAL res;

    /** Points to the storage area for the real part of the orbital */
    REAL *psiR;
    /** Points to the storage area for the imaginary part of the orbital */
    REAL *psiI;


    /** Nuclear potential */
    REAL *vnuc;
    /** Hartree potential */
    REAL *vh;
    /** Exchange correlation potential */
    REAL *vxc;
    /** Total potential */
    REAL *vtot;

    /** Core charge for non-linear core corrections */
    REAL *rhocore;

    /** Grid dimension in the x-coordinate direction on this processor */
    int dimx;
    /** Grid dimension in the y-coordinate direction on this processor */
    int dimy;
    /** Grid dimension in the z-coordinate direction on this processor */
    int dimz;


    /** Grid spacings */
    REAL hxgrid;
    REAL hygrid;
    REAL hzgrid;


    /** Total basis size on each processor (dimx*dimy*dimz) */
    int pbasis;

    /* Total basis size in a smoothing grid on each processor (dimx+2)*(dimy+2)*(dimz+2) */
    int sbasis;

    /*8 Index of the orbital */
    int istate;


    /** Volume element associated with each real space grid point */
    REAL vel;


    /** Occupation of the orbital */
    REAL occupation[2];
    REAL oldeig;

    /* The offsets and the sizes of the grid that the orbital
     * is defined on relative to the global grid. These will
     * be used in the future for cluster boundary condition or
     * localized orbitals in an Order(N) formulation.
     */
    int xoff, yoff, zoff;
    int xsize, ysize, zsize;

    /** Index showing which k-point this orbital is associated with */
    int kidx;


} STATE;


/**@name SPECIES
 * @memo Species (pseudopotential) control structure
 * @doc Structure holds data about the pseudopotentials used to
 * represent different types of atomic species. 
*/
typedef struct
{

    /* symbol read from control file */
    char pseudo_symbol[32];

    /* pseudopotential filename */
    char pseudo_filename[MAX_PATH];

    /** Description of the species (e.g Atomic carbon generated using 
     * hamann's code with  rcs=0.80 rcp=0.85 bohr
     */
    char description[MAX_CHAR];

    /** Atomic number */
    int atomic_number;

    /** Atomic symbol */
    char *atomic_symbol;

    /** Atomic mass */
    REAL atomic_mass;

    /** Number of valence electrons */
    REAL zvalence;

    /** Gaussian charge parameter used for compensating the 1/r Coulomb
     * tail of the pseudopotentials
     */

    REAL rc;

    /* Number of grid points in the local in each coordinate direction. 
     * These used to be L0_LDIM and L0_NLDIM.
     */
    int ldim;
    int nldim;
    int nlfdim;
    int qdim;


    /* These are input parameters in the pseudopotential file. They represent the
     * real radii that are used in generating ldim and nldim.
     */
    REAL lradius;
    REAL nlradius;
    REAL qradius;

    /*Radius for milliken analysis*/
    REAL mill_radius;
    /*Radius in number of grid points*/
    int mill_dim;
    /*Number of radial atomic wave functions - these depend on l only, not on m*/
    int num_atomic_waves;
    /*l-numbers for states for which we have atomic orbitals*/
    int lstate_atomic_wave[5];
    /*Sum of all atomic states (with different l or m numbers*/
    int sum_atomic_waves;

    /*This will store name of atomic wavefunctions, such as s, px, py, pz, dxx etc*/
    char atomic_wave_symbol[20][12];


    /** Number of radial grid points in the pseudopotential file */
    int rg_points;

    /* Log mesh parameter, where aa=exp(-aasf)/Z, bb=1.0/bbsf */
    REAL aa, bb;

    /** Non-linear core correction flag */
    int nlccflag;

    /* Number of potentials */
    int num_potentials;

    /* L-values for the reference states */
    int lval[10];

    /* L-value for local pseudopotential state */
    int local;

    /* Index for local pseudopotential state */
    int localidx;

    /*Number of grid points in the beta function */
    int kkbeta;

    /*matrix ddd0(nbeta,nbeta) */
    REAL ddd0[18][18];
    REAL ddd[18][18];

    /*matrix qqq(nbeta,nbeta) */
    REAL qqq[18][18];

    /*the number of L=|l1-l2|.....|l1+l2|, we limit nlc <=5 */
    int nlc;

    /*the number of component in polynomial of the pseudized Q_I(r) function we limit nqf<=10 */
    int nqf;

    /*L-independent inner coutoff radii rinner for Q_I(r) function */
    REAL rinner[5];

    /* ultrosoft Vanderbilt Qnm_rad(r) function and */
    REAL *qnm;
    REAL *qnmlig;
    REAL *drqnmlig;

    /* the coefficient for pseudosation of Qnm_L(r) */
    REAL *qfcoef;

    /* Logarithmic radial mesh information */
    REAL r[MAX_RGRID];
    REAL rab[MAX_RGRID];


    /* Local Pseudopotentials */
    REAL vloc0[MAX_RGRID];

    /* Core charge radial grids */
    REAL cr[MAX_RGRID];


    /* Pseudo atomic valence density */
    /*REAL avdens[MAX_RGRID];*/
    REAL **atomic_wave;


    /* Pseudo atomic core density */
    REAL rspsco[MAX_RGRID];

    /*the L-value for the beta function */
    int llbeta[MAX_NB];

    /*utrosoft Vanderbilt beta_n(r) function on radial grid */
    REAL beta[MAX_NB][MAX_RGRID];


    /* Total number of projectors */
    int nbeta;


    /* Linear interpolation storage for the compensated local potential
     * and for it's radial derivative.
     */
    REAL localig[MAX_LOCAL_LIG];
    REAL drlocalig[MAX_LOCAL_LIG];

    /* Linear interpolation storage for the core charge density */
    REAL rhocorelig[MAX_LOCAL_LIG];

    /* Utrosoft Vandbelit Projectors on linear interpolation grid */
    REAL betalig[MAX_NB][MAX_LOCAL_LIG];

    /* Radial derivatives of the Utrosoft Vandbelit Projectors on linear interpolation grid */
    REAL drbetalig[MAX_NB][MAX_LOCAL_LIG];

    /* Local potential linear interpolation grid spacing */
    REAL drlig;

    /* Non-local linear interpolation grid spacing */
    REAL drnlig;

    /* Qfunction linear interpolation grid spacing */
    REAL drqlig;


    /* Pseudopotential filtering parameters */
    REAL lrcut;                 /* Real space local cutoff */
    REAL nlrcut[4];             /*Real space nonlocal cutoff */
    REAL rwidth;                /* Real-space width parameter */
    REAL gwidth;                /* G-space width parameter */


    /*Total number (of what exactly ???) */
    int num_projectors;


    /*This will store results of forward fourier transform on the coarse grid */
    fftw_complex *forward_beta;

#if !FDIFF_BETA
    /*This will store results of forward fourier transform for derivatives of beta on the coarse grid */
    fftw_complex *forward_derbeta_x;
    fftw_complex *forward_derbeta_y;
    fftw_complex *forward_derbeta_z;
#endif

    /*Backwards wisdom for fftw */
    char *backward_wisdom;


} SPECIES;


/* Structure for storing species information for internal pseudopotentials */
typedef struct
{
    char name[4];
    REAL valence;
    REAL mass;
    REAL rc;
    int nlccflag;
    int maxl;
    int local;
} ISPECIES;



/*Structure for storing PDB information
 * Each ion should have it*/
typedef struct
{

/* 1 -  6  Record name*/
char record_name[7];

/* 7 - 11 Atom serial number*/
int serial_num;

/*13 - 16  Atom name*/
char name[5];

/* 17 Alternate location indicator.*/
char altLoc[2];

/* 18 - 20 Residue name*/
char resName[4];

/* 22 Chain identifier*/
char chainID[2];

/* 23 - 26 Residue sequence number*/
int resSeq;

/* 27 Code for insertion of residues*/
char iCode[2];

/* 55 - 60 Occupancy*/
REAL occupancy;

/* 61 - 66 Temperature factor*/
REAL tempFactor;

/* 77 - 78  Element symbol, right-justified. */
char element[3];

/*79 - 80  Charge on the atom.*/
char charge[3];

} PDB_INFO;




/* Ion structure */
typedef struct
{

    /* Initial physical coordinates at start of run */
    REAL icrds[3];

    /* Actual Physical coordinates at current time step */
    REAL crds[3];

    /* Positions at the previous time step */
    REAL ocrds[3];

    /* Initial crystal coordinates at start of run */
    REAL ixtal[3];

    /* Actual crystal coordinates at current time step */
    REAL xtal[3];

    /* Crystal coordinates  at the previous time step */
    REAL oxtal[3];

    /*Position of ion relative to the middle of non-local box around the ion 
     *          * determined in get_nlop, AIget_cindex sets this up*/
    REAL nlcrds[3];


    /* Coordinates of the corner of the grid that the local */
    /* difference potential is nonzero on.                  */
    REAL lxcstart;
    REAL lycstart;
    REAL lzcstart;


    /* Coordinates of the corner of the grid that the non-local */
    /* potential is nonzero on.                                 */
    REAL nlxcstart;
    REAL nlycstart;
    REAL nlzcstart;


    /* Coordinates of the corner of the grid that the Qfunction */
    /* potential is nonzero on.                                 */
    REAL Qxcstart;
    REAL Qycstart;
    REAL Qzcstart;


    /* Integer species type when using a raw pseudopotential */
    int species;

    /* Forces on the ion */
    REAL force[4][3];

    /* Current velocity of the ion */
    REAL velocity[3];

    /* Kleinman-Bylander normalization coefficients */
    REAL pd[(MAX_L + 1) * (MAX_L + 1)];

    /* Milliken normalization coefficients */
    REAL mnorm[(MAX_L + 1) * (MAX_L + 1)];

    /* Total number of projectors */
    int prjcount;

    /* Movable flag */
    int movable;

	/* Force modifier parameters */
	struct {
		REAL setA_weight;
		REAL setA_coord[3];
		REAL setB_weight;
		REAL setB_coord[3];
	} constraint;
		

    /* Stored non-local projectors */
    REAL *oldsintR;
    REAL *oldsintI;
    REAL *newsintR;
    REAL *newsintI;
    

    /* Stores sine and cosine of a phase factor for backwards fourier transform */
    REAL *fftw_phase_sin;
    REAL *fftw_phase_cos;


    /*Stores PDB information*/
    PDB_INFO pdb;


} ION;



/* multigrid-parameter structure */
typedef struct
{

    /* number of global-grid pre/post smoothings and timestep */
    REAL gl_step;
    int gl_pre;
    int gl_pst;

    /* timestep for the subiteration */
    REAL sb_step;

    /* timestep for the Richardson-Iteration */
    REAL ri_step;

    /* lowest MG level */
    int levels;


} MG_PARM;

/* Nose control structure */
typedef struct
{

    /* number of atoms allowed to move */
    int N;

    /* ionic target temperature in Kelvin */
    REAL temp;

    /* ionic target kinetic energy */
    REAL k0;

    /* randomize velocity flag */
    int randomvel;

    /* Nose oscillation frequency */
    REAL fNose;

    /* number of thermostats used */
    int m;

    /* thermostat positions,velocities,masses and forces */
    REAL xx[10];
    REAL xv[10];
    REAL xq[10];
    REAL xf[4][10];

} FINITE_T_PARM;


/** @name KPOINT
 * @memo Holds data specific to individual k-points.
 */
typedef struct
{

    /** The index of the k-point for backreferencing */
    int kidx;

    /** The k-point */
    REAL kpt[3];

    /** The corresponding vector */
    REAL kvec[3];

    /** The weight associated with the k-point */
    REAL kweight;

    /** The magnitude of the k-vector */
    REAL kmag;

    /* The orbital structure for this k-point */
    STATE *kstate;


    /* Mean min, and max wavefunction residuals for occupied space */
    REAL meanres;
    REAL minres;
    REAL maxres;

    /* Total energies */
    REAL ES;
    REAL NUC;
    REAL KE;
    REAL XC;
    REAL NL;
    REAL II;
    REAL TOTAL;

} KPOINT;




/** @name CONTROL
  @memo Main control structure
 
  This is a global structure declared as extern CONTROL ct
 
 */
typedef struct
{

#ifdef SMP
    /* Number of threads to run concurrently. We always 
     * use one thread per eigenfunction but it's inefficient 
     * to have more threads than the number of CPU's available
     * running concurrently. Defaults to compile time parameter 
     * of 1 but can be overridden by setting the environment 
     * variable QMD_NUM_THREADS to any value up to MAX_THREADS.
     */
    int thread_concurrency;
    int num_threads;

    /* When running on a cluster this is the total number of nodes
     * in the cluster.
     */
    int num_nodes;

    /* While this is the identity of this node */
    int this_node;

    /* And this is the space offset of this node */
    int node_space_offset;

    /* And this is the number of space points handled by this node */
    int node_space_size;
#endif

    /** Description of the run. */
    char description[MAX_CHAR];

    /* time at which run started */
    REAL time0;
    
    /* determine if this image is processing spin up or spin down. */
    int spin_flag;


    /* determine whether to initialize up and down density equally or not */
    int init_equal_density_flag; 


    /** Name of the input control file. Passed as a command line argument
     *
     *  Example:
     *  bash$  md in.diamond8
     */
    char cfile[MAX_PATH];

    /** HAndle of the output log file. Constructed from command line argument */
    FILE *logfile;

    /** Input file name to read wavefunctions from when doing a restart */
    char infile[MAX_PATH];

    /** Input file name to write wavefunctions to */
    /* Output file name */
    char outfile[MAX_PATH];

    /** File to read the pseudopotentials from */
    /*  char pspfile[MAX_PATH]; */

    /** Initial run flag. Read from the input file. 0=initial run otherwise a restart */
    int runflag;

    /* output z-average of states */
    int zaverage;

    /* number of state to output */
    int plot_state;

    /* Exchage-Correlation flag */
    int xctype;

    /** Boundary condition flag. Read from the input file. 0=periodic, 1=cluster, 2=surface */
    int boundaryflag;

    /* Coordinate input flag: crystal or cartesian */
    int crd_flag;

    /* Maximum number of MD steps */
    int max_md_steps;

    /* Maximum number of rmg meta loops (NEB, ARTS, etc.) */
    int max_rmg_steps;

    /* MD steps iterator */
    int md_steps;

    /* Maximum number of SCF steps in a MD steps */
    int max_scf_steps;

    /* Total number of SCF steps done */
    int total_scf_steps;

    /* SCF steps iterator */
    int scf_steps;

    /* override occupations */
    int override_occ;

    /* Override current ionic positions (in control file) with positions from wave file (during restart) */
    int override_current;

    /* Override initial ionic positions (in control file) with positions from wave file (during restart) */
    int override_initial;

    /* convergence criterion */
    REAL thr_rms;

    /* force convergence criterion */
    REAL thr_frc;

    /* Number of steps after which to perform checkpointing */
    int checkpoint;

    /* Number of steps after which to output results */
    int outcount;

    /** Sorting flag for wavefunctions. Read from input file. 0=no sort, 1=sort */
    int sortflag;

    /** Number of states */
    int num_states;


    /*Number of states for spin up and down used for initialization*/
    int num_states_up, num_states_down;

    /** Number of unoccupied states above Fermi level */
    int num_unocc_states;

    /** string to store repeat count occupations */
    char occupation_str[256];

    /*string to store repeat count occupations for spin up*/
    char occupation_str_spin_up[256];

    /*string to store repeat count occupations for spin down*/
    char occupation_str_spin_down[256]; 
    

    /** Number of ions */
    int num_ions;

    /** Ion structure */
    ION *ions;

    /** Number of species */
    int num_species;

    /* Cutoff parameter */
    REAL cparm;
    REAL betacparm;
    REAL qcparm;

    /** Total conpensating charge density */
    REAL crho;

    /** Total charge in supercell */
    REAL tcharge;

    /** Species structure 
     * @see SPECIES */
    SPECIES *sp;

    /** the fine grid size on each coarse grid cell */
    int nxfgrid;
    int nyfgrid;
    int nzfgrid;

    /** Global uniform grid spacing in x */
    REAL hxgrid;

    /** Global uniform grid spacing in y */
    REAL hygrid;

    /** Global uniform grid spacing in z */
    REAL hzgrid;

    /** The fine uniform grid spacing in x */
    REAL hxxgrid;

    /** The fine uniform grid spacing in y */
    REAL hyygrid;

    /** The fine uniform grid spacing in z */
    REAL hzzgrid;

    /** bravais lattice type */
    int ibrav;

    /** Lattice information */
    REAL celldm[6];

    /* lattice vectors */
    REAL a0[3];
    REAL a1[3];
    REAL a2[3];

    /** Total cell volume */
    REAL omega;

    /* lengths of the sides of the supercell */
    REAL xside;
    REAL yside;
    REAL zside;

    /* This is the max of nldim for any species cubed */
    int max_nlpoints;
    int max_nlfpoints;
    int max_Qpoints;

    /** Maximum grid spacing in any coordinate direction */
    REAL hmaxgrid;


    /** Minimum grid spacing in any coordinate direction */
    REAL hmingrid;


    /** Grid anisotropy defined as the ratio of hmaxgrid to hmingrid. A value larger than 1.05 can lead to convergence problems. */
    REAL anisotropy;


    /** Volume element associated with each grid point */
    REAL vel;
    REAL vel_f;


    /** Physical grid basis size */
    int nbasis;

    /*Flag used to determine whether the actual ionic masses or equal masses set to 
     * that of carbon are to be used in fastrelax algorithm.*/
    int fastrelax_mass;

    /** Density mixing parameter. Typical values range from 0.2 to 0.9, while larger values provide faster convergence as long as they are stable. */
    REAL mix;

    /*Order of Pulay mixing for charge density*/
    int charge_pulay_order;

    /*How often to refresh Pulay history*/
    int charge_pulay_refresh;

    /*Scale parameter for residuals in Pulay mixing*/
    REAL charge_pulay_scale;

    /*Flag to test whether or not the modified metrics should be used in Pulay mixing*/
    int charge_pulay_special_metrics;

    /*Weight for Pulay special metrics*/
    REAL charge_pulay_special_metrics_weight;

    /* Projector mixing parameter */
    REAL prjmix;

    /* Global uniform grid corner */
    REAL xcstart;
    REAL ycstart;
    REAL zcstart;


    /* Hartree potential offset from wavefunction grid */
    int vh_xoffset;
    int vh_yoffset;
    int vh_zoffset;


    /* Hartree potential grid sizes */
    int vh_nxgrid;
    int vh_nygrid;
    int vh_nzgrid;


    /* Hartree potential grid sizes per domain */
    int vh_pxgrid;
    int vh_pygrid;
    int vh_pzgrid;


    /* Total points in hartree potential per domain */
    int vh_pbasis;


    /* Wavefunction grid sizes */
    int psi_nxgrid;
    int psi_fnxgrid;
    int psi_nygrid;
    int psi_fnygrid;
    int psi_nzgrid;
    int psi_fnzgrid;

    /* Total points for wavefunctions */
    int psi_nbasis;
    int psi_fnbasis;

    /* Decoupled hartree potential */
    REAL *vh_ext;


    /* Mean min, and max wavefunction residuals for occupied space */
    REAL meanres;
    REAL minres;
    REAL maxres;

    /* total ionic charge */
    REAL ionic_charge;

    /* Variable occupation stuff */
    REAL nel;

    int occ_flag;

    REAL occ_width;

    REAL occ_mix;

    /** total background smearing charge -- for charged supercells */
    REAL background_charge;


    /** Multigrid parameters for the eigenvalue solver */
    MG_PARM eig_parm;

    /** Multigrid parameters for the poisson solver */
    MG_PARM poi_parm;


    /** Nose paramters */
    FINITE_T_PARM nose;

    /* force pointer array */
    int fpt[4];

    /* temperature control */
    int tcontrol;

    /* md integrate level */
    int mdorder;

    /* movie flags */
    int rmvmovie, chmovie, xbsmovie;

    /* Milliken population flags. */
    int domilliken;
    int milliken;

    /* Diagonalization flag and period */
    int initdiag;
    int diag;
    int end_diag;

    /* How many steps between writeout of eigenvalues*/
    int write_eigvals_period;

    /* Diagonalizations during the md step */
    int mddiag1;
    int mddiag2;

    /** Force flag. 0=don't compute forces, 1=compute forces */
    int forceflag;

    /* Should we process constraints on forces flag */
    int constrainforces;

    /* Whether to write full memory usage report at the end of calculation */
    int write_memory_report;

    /** Ionic motion timestep */
    REAL iondt;


    /** Ionic motion energy */
    REAL ionke;


    /* Total energies */
    REAL ES;
    REAL NUC;
    REAL KE;
    REAL XC;
    REAL NL;
    REAL II;
    REAL TOTAL;

    /* fermi energy */
    REAL efermi;

    /** Total number of k-points being used in the calculation */
    int num_kpts;


    /** K-point control structure */
    KPOINT *kp;

    /** Throttles data transfer rates when writing wavefunctions to disk
     *
     * On clusters with NFS mounted filesystems having all nodes
     * dump there data at the same time can cause network congestion
     * and hangups so wait_flag can be set in the input file to throttle
     * the total bandwidth being written. */
    int wait_flag;

    /** The maximum number of projectors for any species */
    int max_nl;

    /*Maximum value of nldim for any species */
    int max_nldim;

    /*This keeps track whether ct.fftw_wisdom_setup was setup or not so that
     * we know whether to release wisdom memory at the end or not*/
    int fftw_wisdom_setup;


    /*Interpolation flags */
    int interp_flag;

    /*Order of B-spline */
    int interp_order;

    /*Order of trade_image used in interpolation */
    int interp_trade;

    /* the external electric field */
    REAL e_field;

    REAL x_field_0;

    REAL y_field_0;

    REAL z_field_0;

	REAL neb_spring_constant;

} CONTROL;


#ifdef SMP

/* Thread control structures */
typedef struct
{

    /* Thread ID number */
    int tid;

    /* These volatiles are used as synchronization variables for the threads */
    volatile int start;

    /* With the complex option this lets the threads know which k-point is
     * currently being worked on in ortho and subdiag. */
    int kidx;

    /* Pointer to state array used by each thread */
    STATE *my_states;

    /* Local variable -- summed to obtain total charge for all orbitals */
    REAL tcharge;

    /* Spacial offset for the thread */
    int offset;

    /* Points to base of distributed storage array for this thread */
    REAL *base_mem;

    /* Points to base of distributed scratch array for this thread */
    REAL *scratch1;

    /* Number of points per wavefunction in the distributed storage array */
    int numpt;

    /* leading dimension of the distributed wave function storage array */
    int lda;

    /* Local copies of eigenvalues and occupations for this thread */
    REAL *eigs;
    REAL *occs;

    /* Force contributions computed by this thread */
    REAL force[MAX_IONS][3];

    /* Pointer to dynamically allocated arrays of size ct.num_states*ct.num_states */
    /* that is required in ortho. Each thread has it's own copy */
    REAL *darr;
    REAL *barr;


    /* The same array as referenced by darr but this copy is 
     *allocated in the main program rather than in one of the threads.
     */
    REAL *farr;


    REAL *rho;
    REAL *rhocore;
    REAL *vtot;
    REAL *vnuc;

    /* Pointers to the non-local potential index list 
     *and to the projectors themselves */
    int *nlindex;
    REAL *projectors;


} SCF_THREAD_CONTROL;
#endif



/* Extern declaration for the main control structure */
extern CONTROL ct;


/* Extern declaration for the processor control structure */
extern PE_CONTROL pct;


/* Extern declarations for thread control structures */
#ifdef SMP
extern SCF_THREAD_CONTROL thread_control[];
#endif
