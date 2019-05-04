#include<iostream>
#include<math.h>
#include<fstream>
#include<tuple>
#include "config-reader.h"

//constant
constexpr double G = 1;
constexpr double PI = 3.14159265359;

//3D coordinates (x,y,z) or (r,phi,theta);
struct Coord {
    Coord(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    union {
        struct {
            double x, y, z;
        };
        struct {
            double r, phi, theta;
        };
    };

    friend std::ostream &operator<<(std::ostream &os, Coord const &s) {
        os << s.x << ' ' << s.y << ' ' << s.z;
        return os;
    }
};

//Plummer system
struct Plummer {
    Plummer(double M_, double a_, Coord const &p_, Coord const &v_, double t_ = 0) : v(v_), p(p_), M(M_), a(a_),
                                                                                     t(t_) {}

    Coord v;//velocity of the test particle
    Coord p;//position of the teste particle
    double M;//cluster mass
    double a;//cluste scale
    double t;//time of the system

    /**
     * advance the test particle with given time step
     * @param dt the given time step
     * @note I change the integration to second order symplectic method
     */
    void advance_coordinates(double dt) {
        double hdt = 0.5 * dt;//half the time step
        p.x += v.x * hdt;//advance position with half step
        p.y += v.y * hdt;
        p.z += v.z * hdt;

        Coord a = calc_acceleration();//calculate the acceleration in the middle step
        v.x += a.x * dt;//advance the velocity with dt
        v.y += a.y * dt;
        v.z += a.z * dt;

        p.x += v.x * hdt;//advance position with the rest half step
        p.y += v.y * hdt;
        p.z += v.z * hdt;
    }
private:
    /**
     * calculate the acceleration of the test particle
     * @return the acceleration of the teste particle(in 3D coordinates)
     */
    Coord calc_acceleration() {
        double d = sqrt(p.x * p.x + p.y * p.y + p.z * p.z + a * a);

        auto x = -G * M * p.x / (d * d * d);
        auto y = -G * M * p.y / (d * d * d);
        auto z = -G * M * p.z / (d * d * d);
        return Coord(x, y, z);
    }

public:
    /**
     * define how to output the Plummer system
     * @param os the target output stream
     * @param sys the plummer system
     * @return
     */
    friend std::ostream &operator<<(std::ostream &os, Plummer const &sys) {
        os << sys.t << ' ' << sys.p << ' ' << sys.v;
        return os;
    }
};

/**
 * Data structure to store the run parameters
 */
struct Args {
    double end_time;
    double time_step;
    double out_interval;
    std::string file_name;
};

/**
 * subroutine to convert spherical coordinates to cartesian coordiantes
 * @param p spherical postion coordinates
 * @param v spherical velocity coordinates
 * @return the tuple of(cartesian position coordinate, cartesian velocity coordinates)
 */
auto to_cartesian(Coord const &p, Coord const &v) {
    double cos_theta = cos(p.theta);
    double sin_theta = sin(p.theta);
    double cos_phi = cos(p.phi);
    double sin_phi = sin(p.phi);

    auto x = p.r * cos_theta * sin_phi;
    auto y = p.r * sin_theta * sin_phi;
    auto z = p.r * cos_phi;

    auto vx = cos_theta * sin_phi * v.r - p.r * sin_theta * sin_phi * v.theta + p.r * cos_theta * cos_phi * v.phi;
    auto vy = sin_theta * sin_phi * v.r + p.r * sin_phi * cos_theta * v.theta + p.r * sin_theta * cos_phi * v.phi;
    auto vz = cos_phi * v.r - p.r * sin_phi * v.phi;

    return std::make_tuple(Coord(x, y, z), Coord(vx, vy, vz));
}

void calc_angular_momentum(Plummer &sys) {//subroutine to calculate the angular momemtum if we need it
//empty function
}

void evolve(Plummer &sys, Args const &args) {
    std::fstream out_file(args.file_name,
                          std::ios::out);//open the output file with given file name and set it to output mode

    if (out_file.is_open()) {//if the file is open correctly
        double output_time = 0;
        for (; sys.t < args.end_time; sys.t += args.time_step) {

            if (sys.t >= output_time) {//write the plummer system to file if it's the time.
                out_file << sys << '\n';
                output_time += args.out_interval;
            }
            sys.advance_coordinates(args.time_step);//advance the plummer system with given time step
        }
        out_file.close();//close the output file
    } else {//if the file is not oppen correctly
        std::cout << "fail to open the output file!\n";
    }
}

int main() {
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>read parameters from configure file>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    SpaceH::ConfigReader config("config.txt");
    double M = config.get<double>("M");//mass of the cluster
    double a = config.get<double>("a");//scale of the plummer potential
    double r0 = config.get<double>("r0");
    double phi0 = config.get<double>("phi0");
    double theta0 = config.get<double>("theta0");
    double v_r0 = config.get<double>("v_r0");
    double v_phi0 = config.get<double>("v_phi0");
    double v_theta0 = config.get<double>("v_theta0");
    double END_TIME = config.get<double>("end_time");//end time of the integration
    double TIME_STEP = config.get<double>("time_step");//time step of the integration
    double OUTPUT_STEP = config.get<double>("output_step");//output time step
    std::string OUTPUT_FILE = config.get<std::string>("output_file");//name of the output file
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   // CONFIG_MAPPING("config.txt",M, a,r0,phi0,theta0,v_r0,v_phi0,v_theta0,end_time,time_step,out_put_step);

    //convert initial condition from spherical coordinates to cartesian coordinate
    auto[init_pos, init_vel] = to_cartesian(Coord(r0, phi0, theta0), Coord(v_r0, v_phi0, v_theta0));

    Plummer sys(M, a, init_pos, init_vel);//define the plummer system with given M, a, p and v in Cartesian coordinates

    Args args{END_TIME, TIME_STEP, OUTPUT_STEP, OUTPUT_FILE};//define the run parameters

    evolve(sys, args);//evolve the Plummer system
}
