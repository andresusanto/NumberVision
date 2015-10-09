package com.ganesus.numbervision;

public interface TurnCode {
    /**
     * bikin LRLRLRLRRRLRLL dari chain code
     * @param code chain code 0000111213123456712315555
     * @return kode belok
     */
    public String generateTurn(String code);
}
