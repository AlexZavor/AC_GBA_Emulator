#ifndef GBPPU_H
#define GBPPU_H

class gbPPU{
    private:
        gbMEM* MEM;
    public:
        gbPPU(gbMEM* MEM){
            MEM = MEM;
        };
};

#endif /* GBPPU_H */