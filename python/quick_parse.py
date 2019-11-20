import pandas as pd

def select_interaction_sample(df, interaction, n_samples=None):
    df_type = df[df["interaction_type"] == interaction]
    if n_samples is not None:
        df_type = df_type.sample(n = n_samples)
    return df_type

df = pd.read_csv("events.csv")
print(df["interaction_type"].value_counts())
print("Total events:", df["interaction_type"].value_counts().sum())

df_ccqe_mu_p = select_interaction_sample(df, "CCQEL_MU_P", 5)
df_ccres_mu_p_pi = select_interaction_sample(df, "CCRES_MU_P_PIPLUS", 5)

df_ccqe_mu_p.to_csv("sample_ccqe_mu_p.csv", index=False)
df_ccres_mu_p_pi.to_csv("sample_ccres_mu_p_pi.csv", index=False)

"""
CCQEL_MU_P                   33399
CCRES_MU_P_PIPLUS            19111
CCRES_MU_PIPLUS              18201
CCRES_MU_P_PIZERO             8020
CCRES_MU_P                    7349
CCRES_MU_P_P                  5896
CCRES_MU                      5101
CCRES_MU_PIZERO               3630
CCRES_MU_P_P_P                2528
CCRES_MU_P_P_PIPLUS           2145
CCCOH                         2054
CCQEL_MU_P_P                  1949
CCRES_MU_P_P_PIZERO           1348
CCRES_MU_P_P_P_P               961
CCRES_MU_PHOTON                480
CCRES_MU_P_PHOTON              464
CCQEL_E_P                      359
CCQEL_MU_P_P_P                 358
CCRES_MU_P_P_P_P_P             307
CCRES_MU_P_P_P_PIPLUS          299
CCRES_MU_P_P_P_PIZERO          207
CCRES_E_P_PIPLUS               182
CCRES_E_PIPLUS                 180
CCQEL_MU_P_P_P_P               117
CCRES_E_P_PIZERO                80
CCRES_E_P                       80
CCRES_E                         56
CCRES_MU_P_P_P_P_PIZERO         51
CCRES_MU_P_P_P_P_P_PIZERO       27
CCRES_MU_P_P_P_PHOTON           14
CCRES_E_P_PHOTON                 9
"""
