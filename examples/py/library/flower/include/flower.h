#ifndef FLOWER_H
#define FLOWER_H

class Flower {
public:
    /**
     * @brief Computes the area of a circle given its radius
     * @param radius The radius of the circle
     * @return The area of the circle (π * r²)
     */
    double computeCircleArea(double radius);

    /**
     * @brief Computes the volume of a sphere given its radius
     * @param radius The radius of the sphere
     * @return The volume of the sphere (4/3 * π * r³)
     */
    double computeSphereVolume(double radius);

    /**
     * @brief Computes the Fibonacci number at position n
     * @param n The position in the Fibonacci sequence (0-indexed)
     * @return The nth Fibonacci number
     */
    long long computeFibonacci(int n);
};

#endif // FLOWER_H