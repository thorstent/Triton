#include <iostream>
#include <sstream>
#include <stdexcept>

#include "JnbIRBuilder.h"
#include "Registers.h"
#include "SMT2Lib.h"
#include "SymbolicElement.h"


JnbIRBuilder::JnbIRBuilder(uint64_t address, const std::string &disassembly):
  BaseIRBuilder(address, disassembly) {
}


void JnbIRBuilder::imm(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, op1;
  uint64_t          imm   = std::get<1>(this->operands[0]);
  uint64_t          symCF = ap.getRegSymbolicID(ID_CF);

  /* Create the SMT semantic */
  if (symCF != UNSET)
    op1 << "#" << std::dec << symCF;
  else
    op1 << smt2lib::bv(ap.getRegisterValue(ID_CF), 1);

  /* Finale expr */
  expr << smt2lib::ite(
            smt2lib::equal(
              op1.str(),
              smt2lib::bvfalse()),
            smt2lib::bv(imm, REG_SIZE_BIT),
            smt2lib::bv(this->nextAddress, REG_SIZE_BIT));

  /* Create the symbolic element */
  se = ap.createRegSE(expr, ID_RIP, "RIP");

  /* Add the constraint in the PathConstraints list */
  ap.addPathConstraint(se->getID());

  /* Add the symbolic element to the current inst */
  inst.addElement(se);
}


void JnbIRBuilder::reg(AnalysisProcessor &ap, Inst &inst) const {
  OneOperandTemplate::stop(this->disas);
}


void JnbIRBuilder::mem(AnalysisProcessor &ap, Inst &inst) const {
  OneOperandTemplate::stop(this->disas);
}


void JnbIRBuilder::none(AnalysisProcessor &ap, Inst &inst) const {
  OneOperandTemplate::stop(this->disas);
}


Inst *JnbIRBuilder::process(AnalysisProcessor &ap) const {
  this->checkSetup();

  Inst *inst = new Inst(ap.getThreadID(), this->address, this->disas);

  try {
    this->templateMethod(ap, *inst, this->operands, "JNB");
    ap.incNumberOfExpressions(inst->numberOfElements()); /* Used for statistics */
  }
  catch (std::exception &e) {
    delete inst;
    throw;
  }

  return inst;
}
