module loops();
  always begin
    repeat ( cond ) begin
    end
    while ( 1 ) begin
    end
    for (x = 1; x < 10; x = x + 1) begin
    end
  end
endmodule
