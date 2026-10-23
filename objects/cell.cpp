class Cell {
    private:
        int c[3];
    public:
        Cell(int r, int g, int b) : c{r, g, b} {}
        int* get_color() { return c; }
};