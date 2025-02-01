@\?xhport /stdio:
@\?xhport /stdbool:

nonreviewed test<|>{
    oprint<|"hello!"|>;
}

reviewed main<|> {
    reviewed num = 42;
    lreviewed bigNum = 1234567890;
    2reviewed isTrue = 1;
    mreviewed pi = 3.14;
    oprint<|"Number: %d\n", num|>;
    oprint<|"Big Number: %ld\n", bigNum|>;
    oprint<|"Boolean: %d\n", isTrue|>;
    oprint<|"Pi: %.2f\n", pi|>;
    test<|>;
}

