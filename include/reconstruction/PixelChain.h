#pragma once


#include <deque>
#include "graphics/Point.h"


class PixelChain {
    private:
        /*
         * Sequence of pixels, in numerical order, which form a continuous chain.
         */
        std::deque<Point> points;

    public:
        /*
         * Create a new empty chain of pixels.
         */
        PixelChain() {
            this->points.clear();
        }

        /*
         * Create a new chain of pixels which is a copy of <other>.
         * 
         * param other: Reference to another continuous pixel chain.
         */
        PixelChain(const PixelChain& other) {
            // Use the implicit copy constructor for deques.
            this->points = other.points;
        }

        /*
         * Returns the number of pixels in the chain.
         */
        int length() {
            return this->points.size();
        }

        /*
         * Returns the first pixel position in the chain.
         */
        Point head() {
            return this->points.front();
        }

        /*
         * Returns the last pixel position in the chain.
         */
        Point tail() {
            return this->points.back();
        }

        /*
         * Expands the pixel chain by attaching a new pixel to the front.
         * 
         * param point: A pixel location which is adjacent to the current head
         *              of the chain.
         */
        void prepend(Point point) {
            this->points.push_front(point);
        }

        /*
         * Expands the pixel chain by attaching a new pixel to the end.
         *
         * param point: A pixel location which is adjacent to the current tail
         *              of the chain.
         */
        void append(Point point) {
            this->points.push_back(point);
        }

        /*
         * Reverses the order of pixels in this chain, in-place.
         */
        void reverse() {
            std::reverse(this->points.begin(), this->points.end());
        }

        Point get(int index) {
            return this->points.at(index);
        }

        /*
         * Returns a copy of this pixel chain in reverse order. Does not modify
         * this pixel chain.
         */
        PixelChain reversed() {
            PixelChain* copy = new PixelChain(*this);
            copy->reverse();

            return *copy;
        }

        /*
         * Extends this pixel chain by adding an entire other chain to the front.
         * That is, if <other> is a1-a2-...-an and this is b1-b2-...-bm, then
         * this pixel chain will be mutated to become a1-a2-...-an-b1-b2...bm.
         * 
         * The other chain will not be modified.
         * 
         * param other: Another chain of pixels to add to this one.
         */
        void insertFront(PixelChain other) {
            std::deque<Point> copy = other.points;

            for (int i = 0; i < other.length(); i++) {
                this->prepend(copy.front());
                copy.pop_front();
            }
        }

        /*
         * Extends this pixel chain by adding an entire other chain to the back.
         * That is, if <other> is a1-a2-...-an and this is b1-b2-...-bm, then
         * this pixel chain will be mutated to become b1-b2...bm-a1-a2-...-an.
         *
         * The other chain will not be modified.
         *
         * param other: Another chain of pixels to add to this one.
         */
        void insertBack(PixelChain other) {
            std::deque<Point> copy = other.points;

            for (int i = 0; i < other.length(); i++) {
                this->append(copy.back());
                copy.pop_back();
            }
        }
};