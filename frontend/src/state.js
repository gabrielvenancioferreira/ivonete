export const CATEGORIES = [
  { key: "rendaFixa", label: "Renda fixa" },
  { key: "acoes", label: "Acoes" },
  { key: "fiis", label: "FIIs" },
  { key: "exterior", label: "Exterior" },
  { key: "reserva", label: "Reserva" }
];

export const defaultWeights = {
  rendaFixa: 40,
  acoes: 25,
  fiis: 15,
  exterior: 10,
  reserva: 10
};

export const state = {
  users: [],
  monthlyIncome: 0,
  idealPlan: null,
  weights: { ...defaultWeights },
  currentInvestments: {
    rendaFixa: 0,
    acoes: 0,
    fiis: 0,
    exterior: 0,
    reserva: 0
  }
};

export function normalizeNumber(value) {
  const number = Number(value);
  if (!Number.isFinite(number) || number < 0) {
    return 0;
  }
  return number;
}

export function sumObjectValues(obj) {
  return Object.values(obj).reduce((acc, value) => acc + normalizeNumber(value), 0);
}

export function parseAllocationForm(form) {
  const data = new FormData(form);
  return CATEGORIES.reduce((acc, item) => {
    acc[item.key] = normalizeNumber(data.get(item.key));
    return acc;
  }, {});
}

export function getWeightsTotal(weights) {
  return sumObjectValues(weights);
}

export function getSuggestedPlan(monthlyIncome) {
  const income = normalizeNumber(monthlyIncome);
  let profile = "Conservador";
  let investRate = 0.15;
  let suggestedWeights = {
    rendaFixa: 55,
    acoes: 15,
    fiis: 15,
    exterior: 5,
    reserva: 10
  };

  if (income >= 4000 && income < 10000) {
    profile = "Moderado";
    investRate = 0.2;
    suggestedWeights = {
      rendaFixa: 40,
      acoes: 25,
      fiis: 15,
      exterior: 10,
      reserva: 10
    };
  } else if (income >= 10000) {
    profile = "Arrojado";
    investRate = 0.25;
    suggestedWeights = {
      rendaFixa: 30,
      acoes: 35,
      fiis: 15,
      exterior: 15,
      reserva: 5
    };
  }

  return {
    profile,
    investRate,
    monthlyContribution: income * investRate,
    suggestedWeights
  };
}

export function calculateRebalance(weights, currentInvestments, monthlyContribution) {
  const currentTotal = sumObjectValues(currentInvestments);
  const plannedContribution = normalizeNumber(monthlyContribution);
  const projectedTotal = currentTotal + plannedContribution;

  const rows = CATEGORIES.map((category) => {
    const currentValue = normalizeNumber(currentInvestments[category.key]);
    const targetPercent = normalizeNumber(weights[category.key]);
    const targetValue = projectedTotal * (targetPercent / 100);
    const delta = targetValue - currentValue;

    return {
      label: category.label,
      currentValue,
      targetPercent,
      targetValue,
      delta
    };
  });

  const totalPositiveDelta = rows.reduce((acc, row) => acc + Math.max(0, row.delta), 0);
  const totalNegativeDelta = rows.reduce((acc, row) => acc + Math.min(0, row.delta), 0);

  return {
    currentTotal,
    plannedContribution,
    projectedTotal,
    totalPositiveDelta,
    totalNegativeDelta,
    rows
  };
}
